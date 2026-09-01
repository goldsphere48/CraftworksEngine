# Руководство по созданию PipelineManager

Для текущего Clearwater стоит сделать C-style `PipelineManager` без STL-контейнеров и без refcount на первом этапе. Pipeline создаётся при первом запросе, переиспользуется и живёт до остановки renderer.

## 1. Структуры

`pipeline_manager.h`:

```cpp
#pragma once

#include "core/types.h"
#include "renderer_backend.h"

namespace cw::graphics
{
    struct PipelineUniform
    {
        uint32       NameHash;
        UNIFORM_TYPE Type;
        HUniform     BackendUniform;
    };

    struct PipelineResource
    {
        HPipeline        BackendPipeline = nullptr;
        PipelineUniform* Uniforms        = nullptr;
        usize            UniformCount    = 0;
    };

    struct PipelineManager;

    PipelineManager* CreatePipelineManager();

    void DestroyPipelineManager(PipelineManager* manager);

    PipelineResource* GetPipeline(
        PipelineManager* manager,
        const char*      path
    );
}
```

В реализации manager хранит записи:

```cpp
struct PipelineEntry
{
    uint32            PathHash;
    char*             Path;
    PipelineResource* Resource;
};

struct PipelineManager
{
    PipelineEntry* Entries;
    usize          Count;
    usize          Capacity;
};
```

`PipelineResource` выделяется отдельно, поэтому его адрес не изменится при расширении массива `Entries`.

## 2. Поиск pipeline

```cpp
static PipelineResource* FindPipeline(
    PipelineManager* manager,
    const char*      path,
    uint32           pathHash
)
{
    for (usize i = 0; i < manager->Count; ++i)
    {
        PipelineEntry* entry = &manager->Entries[i];

        if (entry->PathHash == pathHash &&
            strcmp(entry->Path, path) == 0)
        {
            return entry->Resource;
        }
    }

    return nullptr;
}
```

Нельзя сравнивать только hash: коллизии маловероятны, но возможны.

На первом этапе линейного поиска достаточно. При нескольких десятках pipelines он будет дешевле и проще полноценной hash table.

## 3. Get-or-create

```cpp
PipelineResource* GetPipeline(
    PipelineManager* manager,
    const char*      path
)
{
    uint32 pathHash = utils::HashString(path);

    PipelineResource* existing =
        FindPipeline(manager, path, pathHash);

    if (existing != nullptr)
    {
        return existing;
    }

    PipelineResource* resource = LoadPipeline(path);
    if (resource == nullptr)
    {
        return nullptr;
    }

    if (manager->Count == manager->Capacity)
    {
        if (!GrowPipelineManager(manager))
        {
            DestroyPipelineResource(resource);
            return nullptr;
        }
    }

    PipelineEntry* entry = &manager->Entries[manager->Count++];
    entry->PathHash = pathHash;
    entry->Path     = CopyString(path);
    entry->Resource = resource;

    return resource;
}
```

`LoadPipeline()` должен содержать логику, которая сейчас находится внутри `material.cpp`:

```text
LoadPipelineAsset
    → ParseVertexFormat
    → ParseUniformType
    → CreatePipeline
    → GetUniform
    → создать PipelineResource
    → FreePipelineAsset
```

Таким образом, только `PipelineManager` знает, как превратить `.ppl` в GPU pipeline.

## 4. Расширение массива

```cpp
static bool GrowPipelineManager(PipelineManager* manager)
{
    usize newCapacity =
        manager->Capacity == 0
            ? 16
            : manager->Capacity * 2;

    PipelineEntry* newEntries =
        new PipelineEntry[newCapacity]{};

    for (usize i = 0; i < manager->Count; ++i)
    {
        newEntries[i] = manager->Entries[i];
    }

    delete[] manager->Entries;

    manager->Entries  = newEntries;
    manager->Capacity = newCapacity;

    return true;
}
```

Позже это можно заменить общим engine allocator.

## 5. Material больше не владеет backend pipeline

```cpp
struct MaterialParameter
{
    float Value[16]{};
    bool  Dirty = true;
};

struct Material
{
    graphics::PipelineResource* Pipeline;
    MaterialParameter*          Parameters;
};
```

Material не должен дублировать `NameHash`, `Type` и `HUniform`. Они одинаковы для всех материалов одного pipeline и находятся в `PipelineResource`.

Загрузка материала:

```cpp
Material* LoadMaterial(
    graphics::PipelineManager* pipelines,
    const char*                materialPath
)
{
    MaterialAsset* asset = assets::LoadMaterialAsset(materialPath);

    PipelineResource* pipeline =
        GetPipeline(pipelines, asset->PipelinePath);

    if (pipeline == nullptr)
    {
        return nullptr;
    }

    Material* material = new Material{};
    material->Pipeline = pipeline;
    material->Parameters =
        new MaterialParameter[pipeline->UniformCount]{};

    // Применить значения из .mat.

    return material;
}
```

Файл материала концептуально должен выглядеть так:

```toml
pipeline = "shaders/solid_color.ppl"

[[parameter]]
name = "u_Color"
value = [1.0, 0.0, 0.0, 1.0]
```

А вызов:

```cpp
LoadMaterial(
    pipelineManager,
    "materials/solid_color.mat"
);
```

## 6. Рисование

```cpp
PipelineResource* pipeline = material->Pipeline;

BindPipeline(pipeline->BackendPipeline);

for (usize i = 0; i < pipeline->UniformCount; ++i)
{
    PipelineUniform* uniform = &pipeline->Uniforms[i];
    MaterialParameter* parameter = &material->Parameters[i];

    UploadUniform(uniform, parameter);
}

DrawMeshBackend(mesh, pipeline->BackendPipeline);
```

## 7. Владение

Рекомендуемый порядок:

```text
GraphicsContext
    owns PipelineManager

PipelineManager
    owns PipelineResource
    owns backend HPipeline

Material
    only references PipelineResource
    owns its parameter values
```

При завершении:

```text
Destroy all materials
Destroy PipelineManager
    → DestroyPipeline для каждой записи
Destroy renderer backend/context
```

На первом этапе не следует удалять pipeline при уничтожении отдельного материала. Они хранятся до `DestroyPipelineManager()`. Это исключает dangling pointers и уже решает повторную компиляцию.

Позже можно добавить:

- refcount;
- hot reload;
- pipeline variants;
- нормализацию путей;
- cache key, учитывающий render state и render-target formats.

Главное изменение сейчас: вынести создание `.ppl → PipelineResource` из `material.cpp` в `pipeline_manager.cpp`, а материалам выдавать ссылку на уже кэшированный `PipelineResource`.
