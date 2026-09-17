# Roadmap рефакторинга модуля рендеринга

Документ описывает поэтапный переход от текущей организации `engine/graphics/`
к двухслойной архитектуре по образцу Defold (`dmGraphics` / `dmRender`).

Ориентир: `D:/Code/Engines/Defold/engine/graphics/` и `.../engine/render/`.
Defold — источник идей по разбиению на слои, а не образец для дословного копирования.

---

## 1. Цель

Сейчас низкоуровневая абстракция устройства и высокоуровневая логика отрисовки
живут в одном каталоге и в одном неймспейсе, ссылаются друг на друга и грузят
файлы с диска. Итог рефакторинга — три слоя с односторонними зависимостями:

```
assets  --->  render  --->  graphics  --->  opengl (backend)
```

Ни одной стрелки назад. Каждый слой можно собрать и протестировать, зная только
слои ниже.

### Диагноз текущего состояния

| # | Проблема | Где видно |
|---|---|---|
| 1 | `GraphicsContext` держит `PipelineManager*` и `MaterialContext*` — контекст устройства знает про материалы | `graphics/renderer.h:25-31` |
| 2 | Взаимные зависимости: `renderer.h` форвардит менеджеры, `pipeline_manager.cpp` включает `renderer.h` | `renderer.h:9-10`, `pipeline_manager.cpp:5` |
| 3 | Две функции `DrawMesh` в `cw::gfx` и `cw::graphics` — симптом неразведённых слоёв | `gfx.cpp:67`, `renderer.cpp:148` |
| 4 | Загрузка с диска зашита в графику: `AcquirePipeline(mgr, path)`, `LoadMaterial(ctx, path)` | `pipeline_manager.cpp:186`, `material.cpp:66` |
| 5 | Uniform-модель повторяет OpenGL global state: bind + по одному сеттеру на значение | `gfx.cpp:20-64` |
| 6 | `Mesh` объявлен в заголовке уровня устройства, хотя бэкенд про меши не знает | `renderer.h:13-18` vs `renderer_backend.h:96` |
| 7 | Отрисовка чисто immediate: нет списка, сортировки, батчинга, проходов | `gfx.cpp:67-72` |
| 8 | `HUniformBuffer` объявлен в API, но не реализован и не используется | `renderer.h:61-67` |

### Принципы, которые удерживаем на всех этапах

- Процедурный C-style API: свободные функции, plain-структуры, явное состояние.
- Никаких владеющих контейнеров STL и умных указателей. `darray` — свой.
- У каждого ресурса один владелец и явный путь разрушения.
- Ноль скрытых аллокаций в коде, исполняемом каждый кадр.
- `gl*` и GL-типы не покидают `engine/graphics/opengl/`.
- Каждая фаза заканчивается компилируемым, работающим состоянием и коммитом.

---

## 2. Целевая структура

```
engine/graphics/                     // cw::graphics — устройство.
    graphics.h                       // публичный API: контекст, ресурсы, draw
    graphics.cpp
    graphics_adapter.h               // таблица функций бэкенда
    graphics_types.h                 // хендлы, enum'ы, *Desc-структуры
    opengl/
        opengl_graphics.h
        opengl_graphics.cpp
        opengl_platform_win32.cpp
        gl_functions.h
        gl_functions.cpp
    null/
        null_graphics.cpp            // бэкенд-заглушка для тестов

engine/render/                       // cw::render — рендерер.
    render.h                         // RenderContext, RenderObject, кадр
    render.cpp
    pipeline.h                       // render::Pipeline: HPipeline + рефлексия uniform'ов
    pipeline.cpp
    material.h                       // render::Material: Pipeline* + значения
    material.cpp
    constant_buffer.h                // именованные константы, заливка пачкой
    constant_buffer.cpp
    mesh.h
    mesh.cpp

engine/assets/                       // пути, парсинг, кэш, refcount
    assets_manager.h/.cpp            // парсинг toml (как сейчас)
    resource_cache.h/.cpp            // общий кэш path -> resource + refcount
    res_pipeline.cpp                 // .pso -> render::CreatePipeline
    res_material.cpp                 // .mat -> render::CreateMaterial
```

### Соответствие «было → стало»

| Сейчас | Станет | Аналог в Defold |
|---|---|---|
| `graphics/renderer_backend.h` | `graphics/graphics_adapter.h` + `graphics_types.h` | `graphics/src/graphics_adapter.h` |
| `graphics/renderer.{h,cpp}` | `graphics/graphics.{h,cpp}` | `graphics/src/graphics.{h,cpp}` |
| `struct RenderBackend` | `struct GraphicsAdapter` | `GraphicsAdapterFunctionTable` |
| `graphics/opengl/opengl_renderer.*` | `graphics/opengl/opengl_graphics.*` | `graphics/src/opengl/` |
| `graphics/opengl/opengl_windows.cpp` | `graphics/opengl/opengl_platform_win32.cpp` | `graphics/src/opengl/*_platform.cpp` |
| `graphics/gfx.{h,cpp}` | `render/render.{h,cpp}` | `render/src/render/render.cpp` |
| `graphics/material.{h,cpp}` | `render/material.{h,cpp}` | `render/src/render/material.cpp` |
| `graphics/pipeline_manager.*` | `render/pipeline.*` + `assets/res_pipeline.cpp` | `render/.../material.cpp` + `gamesys/res_material.cpp` |
| `PipelineResource` | `render::Pipeline` | `dmRender::Material` |
| `Mesh` в `renderer.h` | `render/mesh.{h,cpp}` | — |
| `HUniform { void* }` | `HUniformLocation` (int32) | `dmGraphics::HUniformLocation` |
| `SetFloat/Vec2/Vec3/Vec4/Mat4` | `SetConstantV4` / `SetConstantM4` | одноимённые |
| — | `render::RenderObject` + список за кадр | `dmRender::RenderObject`, `RenderListEntry` |

### Про имена `Pipeline` и `Program`

Defold разделяет `HProgram` (шейдерная программа) и `HVertexDeclaration`
(разметка вершин). У нас `PipelineDesc` уже объединяет программу и атрибуты —
это ближе к Vulkan-пайплайну, и это правильное направление. Поэтому:

- `graphics::HPipeline` — программа + вершинная разметка + (позже) fixed-function state.
- `render::Pipeline` — `HPipeline` + рефлексия uniform'ов (имена, хеши, локации, типы).
- `render::Material` — `Pipeline*` + значения параметров.

Имена `HPipeline` / `PipelineDesc` не трогаем, переименовывается только
`PipelineResource` → `render::Pipeline`.

---

## 3. Фазы

Порядок выбран так, чтобы каждый шаг был механическим и обратимым, а рискованные
изменения семантики шли после того, как границы модулей уже разведены.

| Фаза | Суть | Риск | Объём |
|---|---|---|---|
| 0 | Починить существующие баги | низкий | ~30 строк |
| 1 | Переименование слоя graphics, очистка `GraphicsContext` | низкий, механический | ~10 файлов |
| 2 | Выделить `engine/render/` | низкий, механический | перенос файлов |
| 3 | Вынести загрузку ресурсов в `assets/` | средний | новый слой |
| 4 | Uniform locations + константы пачкой | средний | затрагивает бэкенд |
| 5 | `RenderObject` + список за кадр | средний | новая логика |
| 6 | Vertex declaration, render state, второй бэкенд | — | по мере надобности |

---

### Фаза 0. Починить то, что сломано сейчас

Делаем до рефакторинга, чтобы не тащить баги через переносы и не путать
регрессии от рефакторинга с уже существующими дефектами.

**0.1. Опечатка в проверке зарезервированных имён.**
`pipeline_manager.cpp:112`:

```cpp
for (usize j = 0; i < BUILTIN_UNIFORM_COUNT; ++j)   // <- i вместо j
```

При `i == 0` тело не выполняется ни разу, при `i > 0` — выход за границы
`g_BuiltinUniforms` и незавершающийся цикл. Проверка на конфликт с `u_Model`
сейчас не работает. Исправить на `j < BUILTIN_UNIFORM_COUNT`.

**0.2. Встроенные uniform'ы теряются.**
`CreatePipelineFromAsset` создаёт пайплайн с `asset->UniformsCount + BUILTIN_UNIFORM_COUNT`
описаниями (`pipeline_manager.cpp:155-176`), но затем:

```cpp
resource->UniformsCount = asset->UniformsCount;   // pipeline_manager.cpp:214
```

и заполнение идёт только по `asset->UniformsInfo`. `u_Model` компилируется в
шейдер, но в `PipelineResource` не попадает — установить его невозможно.

Решение — держать встроенные uniform'ы отдельным массивом фиксированной длины,
а не мешать их с пользовательскими:

```cpp
struct Pipeline
{
    HPipeline        BackendPipeline = {};
    HUniformLocation Builtins[BUILTIN_UNIFORM_COUNT] = {};  // u_Model, позже u_View/u_Projection
    PipelineUniform* Uniforms        = nullptr;             // только объявленные в .pso
    usize            UniformsCount   = 0;
};
```

Так `Material::Parameters` остаётся строго параллельным `Pipeline::Uniforms`
(на это опирается цикл в `gfx.cpp:22-28`), и никакой арифметики со смещением
индексов не появляется. Альтернатива — один общий массив с зарезервированным
префиксом `[0, BUILTIN_UNIFORM_COUNT)` — тоже рабочая, но требует помнить про
смещение в каждом месте, где индексируются параметры. Выбрать один вариант и
зафиксировать комментарием у структуры.

**0.3. Висячие указатели при разрушении менеджера.**
`DestroyPipelineManager` (`pipeline_manager.cpp:70-85`) разрушает ресурсы, но
оставляет указатели в `Entries`. Течи нет — `darray` освободит буфер в
деструкторе, — но состояние между циклом и `delete manager` некорректно.
Добавить `manager->Entries.Clear()` после цикла.

**Готовность фазы:** сборка зелёная, треугольник рисуется, `ctest` проходит.
**Коммит:** `Fix pipeline manager uniform handling and validation loop.`

---

### Фаза 1. Слой graphics: переименование и очистка контекста

Цель — чтобы `engine/graphics/` описывал **только устройство** и не знал ни про
материалы, ни про меши, ни про пайплайн-менеджер.

**1.1. Переименование файлов и типов.**

```
renderer_backend.h             -> graphics_adapter.h   (типы функций + таблица)
                               -> graphics_types.h     (хендлы, enum'ы, *Desc)
renderer.h / renderer.cpp      -> graphics.h / graphics.cpp
opengl/opengl_renderer.{h,cpp} -> opengl/opengl_graphics.{h,cpp}
opengl/opengl_windows.cpp      -> opengl/opengl_platform_win32.cpp
```

Разделение `renderer_backend.h` на два файла нужно потому, что `assets/` и
`render/` должны видеть `VERTEX_FORMAT`, `UNIFORM_TYPE`, `HPipeline`, но не
обязаны видеть таблицу указателей на функции бэкенда.

Переименования типов:

```
RenderBackend        -> GraphicsAdapter
RENDER_BACKEND_TYPE  -> ADAPTER_FAMILY   (RENDER_BACKEND_OPENGL -> ADAPTER_FAMILY_OPENGL)
GetGLBindings        -> GetGLAdapter
```

**1.2. Очистить `GraphicsContext`.**

```cpp
// graphics.h
struct GraphicsContext
{
    GraphicsAdapter Adapter;
    Viewport        Viewport;
};
```

`PipelineManager*` и `MaterialContext*` временно переезжают в `Engine`:

```cpp
// engine/engine/engine.h
struct Engine
{
    platform::PlatformContext* Platform        = nullptr;
    graphics::GraphicsContext* Graphics        = nullptr;
    graphics::PipelineManager* PipelineManager = nullptr;   // уйдёт в фазе 2
    graphics::MaterialContext* MaterialContext = nullptr;   // уйдёт в фазе 2
    bool                       IsRunning       = true;
};
```

Создание и разрушение переносится из `graphics::Create` / `graphics::Destroy`
(`renderer.cpp:37-38`, `renderer.cpp:48-49`) в `CreateEngine` / `DestroyEngine`
(`engine.cpp:34`, `engine.cpp:72`). Порядок разрушения: материалы → пайплайны →
графика.

Это промежуточное состояние живёт ровно одну фазу — в фазе 2 оба указателя
поглотит `RenderContext`.

**1.3. Убрать `Mesh` из `graphics.h`.**
`struct Mesh` и `CreateMesh` / `DestroyMesh` / `DrawMesh` сразу выносятся в
`engine/render/mesh.{h,cpp}` — так файл трогается один раз, а не дважды.

Слой устройства оставляет только:

```cpp
void Draw(GraphicsContext* ctx, const DrawCall* draw);
```

`DrawCall` уже описан правильно (`renderer_backend.h:96-103`) — бэкенд не знает,
что такое меш.

**1.4. Обновить `CMakeLists.txt`.**

```cmake
  engine/graphics/graphics.cpp
  engine/graphics/opengl/opengl_graphics.cpp
  engine/graphics/opengl/opengl_platform_win32.cpp
  engine/graphics/opengl/gl_functions.cpp
```

**1.5. (Рекомендуется) Null-бэкенд.**
Добавить `engine/graphics/null/null_graphics.cpp` — таблица функций-заглушек,
возвращающих валидные фиктивные хендлы. Это:

- проверяет, что абстракция адаптера действительно backend-neutral;
- позволяет тестам в `tests/` создавать `GraphicsContext` без окна и GL-контекста;
- в Defold ровно для этого существует `graphics/src/null/`.

Выбор бэкенда по `ADAPTER_FAMILY` в `BindBackend` (`renderer.cpp:9-21`) уже
готов принять второй вариант.

**Готовность фазы:** в `engine/graphics/*.h` не встречаются слова `Material`,
`Mesh`, `PipelineManager`; grep по `gl` вне `opengl/` пуст; игра работает.
**Коммит:** `Rename renderer layer to graphics, decouple device context.`

---

### Фаза 2. Выделить `engine/render/`

Механический перенос: файлы, неймспейс, инклюды. Логику не трогаем.

**2.1. Перенос файлов.**

```
graphics/gfx.{h,cpp}              -> render/render.{h,cpp}
graphics/material.{h,cpp}         -> render/material.{h,cpp}
graphics/pipeline_manager.{h,cpp} -> render/pipeline.{h,cpp}
```

Неймспейс всех перенесённых файлов: `cw::render`. Неймспейс `cw::gfx` исчезает.
Коллизия `gfx::DrawMesh` / `graphics::DrawMesh` разрешается сама: остаются
`render::DrawMesh` и `graphics::Draw`.

Переименования:

```
PipelineResource -> render::Pipeline
PipelineManager  -> render::PipelineRegistry   (умрёт в фазе 3)
MaterialContext  -> поглощается RenderContext
```

**2.2. Ввести `RenderContext`.**

```cpp
// render/render.h
namespace cw::render
{
    struct RenderContext;

    struct RenderParams
    {
        graphics::GraphicsContext* Graphics;
        usize MaxRenderObjects;   // всё преаллоцировано заранее
    };

    RenderContext* CreateRenderContext(const RenderParams* params);
    void DestroyRenderContext(RenderContext* ctx);

    graphics::GraphicsContext* GetGraphicsContext(RenderContext* ctx);

    void BeginFrame(RenderContext* ctx);
    void EndFrame(RenderContext* ctx);
}
```

```cpp
// render/render.cpp
struct RenderContext
{
    graphics::GraphicsContext* Graphics;
    PipelineRegistry*          Pipelines;
    // фаза 5: RenderObject* Objects; usize ObjectCount; usize ObjectCapacity;
};
```

`Engine` теряет два временных поля и получает одно:

```cpp
struct Engine
{
    platform::PlatformContext* Platform  = nullptr;
    graphics::GraphicsContext* Graphics  = nullptr;
    render::RenderContext*     Render    = nullptr;
    bool                       IsRunning = true;
};
```

**2.3. Обновить `game/game.cpp`.**

```cpp
g_State.Mesh     = render::CreateMesh(engine->Render, vertices, sizeof(vertices), indices, 3);
g_State.Material = render::LoadMaterial(engine->Render, "materials/blue.mat");
...
render::DrawMesh(engine->Render, g_State.Material, g_State.Mesh);
```

Игровой код больше не трогает `engine->Graphics` вообще. Это хороший индикатор:
если после фазы 2 в `game/` осталось обращение к `cw::graphics`, значит что-то
не доехало до слоя render.

**2.4. `CMakeLists.txt`:** добавить `engine/render/*.cpp`, убрать старые пути.

**Готовность фазы:** `grep -rn "cw::gfx" .` пуст; в `engine/render/` обращения к
`cw::graphics` только к API устройства (хендлы, `Create*`, `Draw`, `Bind*`).
**Коммит:** `Extract render layer into engine/render.`

---

### Фаза 3. Вынести загрузку ресурсов в `assets/`

Сейчас `render` знает про пути к файлам и сам держит кэш. После фазы `render`
принимает только уже разобранные описания.

**3.1. Новый API render-уровня — без путей.**

```cpp
// render/pipeline.h
struct PipelineUniformDesc
{
    const char*  Name;
    UNIFORM_TYPE Type;
};

struct PipelineCreateDesc
{
    const char*                      DebugName;      // путь, только для сообщений об ошибках
    const char*                      VertexSource;
    const char*                      FragmentSource;
    const graphics::VertexAttribute* Attributes;
    usize                            AttributeCount;
    const PipelineUniformDesc*       Uniforms;
    usize                            UniformCount;
};

Pipeline* CreatePipeline(RenderContext* ctx, const PipelineCreateDesc* desc);
void      DestroyPipeline(RenderContext* ctx, Pipeline* pipeline);
```

```cpp
// render/material.h
struct MaterialParamDesc
{
    const char*  Name;
    const float* Values;
    usize        ValueCount;
};

Material* CreateMaterial(RenderContext* ctx,
                         const Pipeline* pipeline,
                         const MaterialParamDesc* params,
                         usize paramCount);
void      DestroyMaterial(Material* material);
```

Валидация (`ValidateUniformNames`, проверка количества значений, поиск uniform'а
по имени) остаётся в render — это правила самого рендерера, а не формата файла.
Путь к файлу приходит в `DebugName`, чтобы сообщения об ошибках не потеряли
контекст.

**3.2. Общий кэш ресурсов.**

```cpp
// assets/resource_cache.h
typedef void (*FDestroyResource)(void* userContext, void* resource);

struct ResourceCache;

ResourceCache* CreateResourceCache(void* userContext, FDestroyResource destroy);
void  DestroyResourceCache(ResourceCache* cache);

void* CacheFind(ResourceCache* cache, const char* path);       // +1 к refcount
bool  CacheInsert(ResourceCache* cache, const char* path, void* resource);
void  CacheRelease(ResourceCache* cache, void* resource);      // -1, при 0 — destroy
```

Сюда переезжают `CopyString`, поиск по хешу пути и линейный `strcmp`-фолбэк из
`pipeline_manager.cpp:37-42, 88-101`. Внутри — `darray` записей. Именно этим в
Defold занимается `resource/`, а не `render/`.

**3.3. Resource-обёртки.**

```cpp
// assets/res_pipeline.cpp
render::Pipeline* AcquirePipeline(AssetContext* assets, const char* path)
{
    if (void* cached = CacheFind(assets->PipelineCache, path))
        return static_cast<render::Pipeline*>(cached);

    PipelineAsset* asset = LoadPipelineAsset(path);      // toml, как сейчас
    if (asset == nullptr)
        return nullptr;

    PipelineCreateDesc desc = {};
    desc.DebugName = path;
    // ... перекладываем поля asset -> desc
    render::Pipeline* pipeline = render::CreatePipeline(assets->Render, &desc);

    FreePipelineAsset(asset);

    if (pipeline == nullptr || !CacheInsert(assets->PipelineCache, path, pipeline))
    {
        render::DestroyPipeline(assets->Render, pipeline);
        return nullptr;
    }

    return pipeline;
}
```

Аналогично `res_material.cpp`: грузит `.mat`, через `AcquirePipeline` получает
пайплайн, вызывает `render::CreateMaterial`.

**3.4. Кто кого зовёт.**
`game.cpp` переключается с `render::LoadMaterial(...)` на
`assets::AcquireMaterial(engine->Assets, "materials/blue.mat")`. В `Engine`
появляется `assets::AssetContext* Assets`, создаваемый после `Render`.

**Готовность фазы:** в `engine/render/` нет ни одного `#include "assets/..."`
и ни одного `fs::`; повторная загрузка того же `.mat` не создаёт второй пайплайн.
**Коммит:** `Move resource loading out of render into assets layer.`

---

### Фаза 4. Uniform locations и константы пачкой

Первое изменение, снимающее OpenGL-специфику с самого интерфейса.

**4.1. Заменить хендл uniform'а на локацию.**

```cpp
// graphics_types.h
typedef int32 HUniformLocation;
constexpr HUniformLocation INVALID_UNIFORM_LOCATION = -1;
```

`HUniform { void* Id; }` (`renderer_backend.h:51-54`) удаляется. В GL-бэкенде
локация — результат `glGetUniformLocation`; в будущем Vulkan-бэкенде — смещение
внутри UBO. Ни то ни другое не требует выделять объект на каждый uniform, как
сейчас делает `GLUniform` (`opengl_renderer.cpp:13-17`).

**4.2. Сократить таблицу адаптера.**
Убрать `FSetFloat`, `FSetVec2`, `FSetVec3`, `FSetVec4`, `FSetMat4`
(`renderer_backend.h:130-140`). Вместо них:

```cpp
typedef void (*FSetConstantV4)(HPipeline pipeline, const float* data, usize count, HUniformLocation loc);
typedef void (*FSetConstantM4)(HPipeline pipeline, const float* data, usize count, HUniformLocation loc);
```

GL-реализация — `glUniform4fv(loc, count, data)` и
`glUniformMatrix4fv(loc, count, GL_FALSE, data)`. Все скалярные и векторные типы
поднимаются до `vec4` при хранении: так делает Defold, и это ровно то, что нужно
UBO-раскладке в Vulkan.

Следствие: `switch` из пяти веток в `BindMaterial` (`gfx.cpp:29-63`) схлопывается
в две, а `memcpy` в `Mat4` (`gfx.cpp:56-59`) исчезает.

**4.3. `ConstantBuffer`.**

```cpp
// render/constant_buffer.h
struct ConstantBuffer;

ConstantBuffer* CreateConstantBuffer(usize maxConstants);
void DestroyConstantBuffer(ConstantBuffer* cb);

void SetConstant(ConstantBuffer* cb, uint64 nameHash, const Vec4* values, usize count);
bool GetConstant(const ConstantBuffer* cb, uint64 nameHash, const Vec4** outValues, usize* outCount);
void ClearConstants(ConstantBuffer* cb);

// Заливает все константы буфера в связанный пайплайн одним проходом.
void ApplyConstants(RenderContext* ctx, const Pipeline* pipeline, const ConstantBuffer* cb);
```

`Material` хранит значения в том же представлении (`Vec4`-слоты вместо
`float Value[16]`), а `ConstantBuffer` на `RenderObject` даёт per-instance
переопределения — то, чего сейчас нет вовсе и из-за чего закомментирован
`SetVec4` в `game.cpp:39`.

**Готовность фазы:** в `engine/graphics/graphics.h` нет
`SetUniformFloat/Vec2/Vec3/Vec4/Mat4`; цвет треугольника меняется через
`SetConstant` из `CW_AppUpdate`.
**Коммит:** `Replace per-uniform setters with batched constants.`

---

### Фаза 5. `RenderObject` и список отрисовки

До сих пор всё было перекладыванием кода. Здесь меняется модель отрисовки:
с немедленной на «накопить за кадр → отсортировать → отправить».

**5.1. Структуры.**

```cpp
// render/render.h
struct RenderObject
{
    const Material* Material       = nullptr;
    const Mesh*     Mesh           = nullptr;
    ConstantBuffer* Constants      = nullptr;   // необязательные переопределения
    Mat4            WorldTransform = {};
    uint32          BatchKey       = 0;         // hash(pipeline, mesh) — для группировки
    uint32          Order          = 0;
    uint8           MajorOrder     = 0;         // проход: before / world / after
};

void AddRenderObject(RenderContext* ctx, const RenderObject* ro);
```

**5.2. Кадр.**

```cpp
void BeginFrame(RenderContext* ctx)
{
    ctx->ObjectCount = 0;                    // без аллокаций: буфер преаллоцирован
    graphics::BeginFrame(ctx->Graphics);
}

void DrawRenderList(RenderContext* ctx, const Camera* camera)
{
    SortRenderObjects(ctx);                  // ключ: (MajorOrder, BatchKey, Order)

    const Pipeline* boundPipeline = nullptr;
    const Material* boundMaterial = nullptr;

    for (usize i = 0; i < ctx->ObjectCount; ++i)
    {
        const RenderObject& ro = ctx->Objects[i];

        if (ro.Material->Pipeline != boundPipeline)
        {
            boundPipeline = ro.Material->Pipeline;
            graphics::BindPipeline(ctx->Graphics, boundPipeline->BackendPipeline);
            ApplyCameraConstants(ctx, boundPipeline, camera);   // u_View, u_Projection
            boundMaterial = nullptr;
        }

        if (ro.Material != boundMaterial)
        {
            boundMaterial = ro.Material;
            ApplyMaterialConstants(ctx, boundPipeline, boundMaterial);
        }

        ApplyBuiltinModel(ctx, boundPipeline, &ro.WorldTransform);

        if (ro.Constants != nullptr)
        {
            ApplyConstants(ctx, boundPipeline, ro.Constants);
        }

        const DrawCall draw = MakeDrawCall(ro);
        graphics::Draw(ctx->Graphics, &draw);
    }
}
```

Ключевой выигрыш помимо структуры: сейчас `BindMaterial` (`gfx.cpp:14`)
перевыставляет **все** uniform'ы на каждый вызов `DrawMesh`, даже когда материал
не менялся. Отслеживание `boundPipeline` / `boundMaterial` это убирает.

**5.3. Буфер объектов.**
`MaxRenderObjects` из `RenderParams` выделяется один раз при создании контекста.
При переполнении — `CW_ERROR` и отбрасывание объекта, без реаллокации в кадре.
Сортировка — на месте, по заранее выделенному массиву индексов.

**5.4. Камера.**
`Camera` (`gfx.h:8-12`) переезжает в `render/render.h`, `u_View` и `u_Projection`
добавляются в `g_BuiltinUniforms` рядом с `u_Model`.

`CW_AppUpdate` становится:

```cpp
render::RenderObject ro = {};
ro.Material       = g_State.Material;
ro.Mesh           = g_State.Mesh;
ro.WorldTransform = mat4::Identity();
render::AddRenderObject(engine->Render, &ro);
```

а вызов `DrawRenderList` — в `UpdateEngine` между `CW_AppUpdate` и `EndFrame`.

**Готовность фазы:** два объекта с одним материалом дают один `BindPipeline` на
кадр; порядок отрисовки предсказуем и управляется `Order`.
**Коммит:** `Introduce render objects and per-frame render list.`

---

### Фаза 6. Дальше, по мере надобности

Не входит в основной рефакторинг, но структура должна это принять без ломки:

- **Vertex declaration отдельным ресурсом.** Сейчас атрибуты вшиты в
  `PipelineDesc` и оседают в VAO внутри `GLPipeline` (`opengl_renderer.cpp:19-26`).
  Для Vulkan нужен `HVertexDeclaration`, создаваемый отдельно и связываемый с
  буфером: `NewVertexDeclaration(ctx, streams)` + `EnableVertexDeclaration(...)`.
- **Render state в `PipelineDesc`.** Blend, depth, cull, scissor, face winding.
  Класть в описание пайплайна (Vulkan-way), **не** делать `EnableState` / `DisableState`.
- **Render targets и проходы.** `HRenderTarget`, `SetRenderTarget`, затем
  предикаты / теги материалов для выборки объектов в проход.
- **Второй бэкенд.** Vulkan или DX12. Если null-бэкенд из фазы 1 существует и
  тесты на нём зелёные — абстракция, скорее всего, выдержит.

---

## 4. Инварианты, которые проверяем после каждой фазы

```sh
# 1. GL не протекает за пределы бэкенда
grep -rn "\bgl[A-Z]\|GLuint\|GLenum\|GL_" engine --include=*.h --include=*.cpp \
  | grep -v "engine/graphics/opengl/"        # должно быть пусто

# 2. render не знает про файлы и ассеты
grep -rn "assets::\|fs::\|LoadPipelineAsset\|LoadMaterialAsset" engine/render/   # пусто

# 3. graphics не знает про высокий уровень
grep -rn "Material\|Mesh\|PipelineRegistry\|RenderObject" engine/graphics/       # пусто

# 4. игра не дёргает устройство напрямую
grep -rn "cw::graphics\|graphics::" game/                                        # пусто
```

Плюс каждый раз: сборка обеих конфигураций, `ctest`, запуск `game` — треугольник
на месте.

---

## 5. Чего из Defold сознательно не берём

| Что | Почему |
|---|---|
| `EnableState` / `DisableState` (GL-style state machine) | Историческое наследие Defold; для Vulkan/DX12 состояние должно жить в пайплайне |
| Render script на Lua | Нет скриптового слоя и не планируется в этом виде |
| Разделение `dmsdk/` (публичные) и приватных заголовков | Оверинжиниринг, пока нет внешних расширений |
| `HOpaqueHandle` и generic handle-контейнер | Свои типизированные хендлы проще и достаточны |
| Теги материалов и предикаты | Появятся вместе с проходами, не раньше |
| `RenderListDispatch` с колбэками по типам компонентов | Нужен, когда появятся разные типы рендерящихся компонентов (спрайты, модели, GUI) |

---

## 6. Сводный чеклист

- [x] **Фаза 0** — исправлен цикл валидации, встроенные uniform'ы доезжают до `Pipeline`, `Entries.Clear()`
- [x] **Фаза 1** — `renderer* -> graphics*`, `GraphicsContext` очищен, null-бэкенд, CMake обновлён
- [x] **Фаза 2** — создан `engine/render/`, неймспейс `cw::render`, `RenderContext`, `game.cpp` не видит `cw::graphics`
- [x] **Фаза 3** — `resource_cache`, `res_pipeline.cpp`, `res_material.cpp`; `render` не грузит файлы
- [x] **Фаза 4** — `HUniformLocation`, `SetConstantV4/M4`, `ConstantBuffer`
- [x] **Фаза 5** — `RenderObject`, список за кадр, сортировка, отсечение повторных биндов, камера
- [ ] **Фаза 6** — vertex declaration, render state, render targets, второй бэкенд

Фазы 1 и 2 механические и дают основную часть выигрыша по читаемости — их разумно
сделать одним заходом, а дальше идти по одной фазе за раз.

---

## 7. Отступления от плана, принятые при реализации

Фазы 0-5 реализованы. Четыре места, где итог отличается от написанного выше:

**7.1. Фазы 1 и 2 сделаны одним заходом.** Как и рекомендовал сам документ.
Промежуточное состояние с `PipelineManager*` / `MaterialContext*` в `Engine`
не создавалось: `Mesh` сразу уехал в `render/mesh.{h,cpp}`, а оба менеджера
сразу поглотил `RenderContext`.

**7.2. `HUniformLocation` — непрозрачный индекс, а не GL-локация.** Бэкенд
выдаёт индекс в собственную таблицу uniform'ов, где рядом с настоящей
GL-локацией лежит объявленный тип. Это то, что позволяет адаптеру иметь ровно
две точки входа (`SetConstantV4` / `SetConstantM4`), не ломая `uniform float` и
`uniform vec3`: тип разворачивает бэкенд в нужный `glUniform{1,2,3,4}fv`.
Defold так может позволить себе только vec4/mat4, потому что его shaderc
переписывает шейдеры; у нас такого шага нет. Для Vulkan индекс станет смещением
в UBO — семантика не протекает наружу в обоих случаях.

**7.3. `DrawRenderList(ctx)` без параметра камеры.** Камера живёт в
`RenderContext` и ставится через `SetCamera(ctx, &camera)`. Причина: цикл кадра
в `engine.cpp` своей камеры не имеет, а игра получает `const Engine*` и не может
положить камеру в `Engine`. Подпись из раздела 5.2 потребовала бы либо
дублирования камеры, либо `DrawRenderList(ctx, GetCamera(ctx))`.

**7.4. Попутно.** Удалены мёртвые объявления `HUniformBuffer` и
`CreateUniformBuffer` / `DestroyUniformBuffer` / `BindUniformBuffer` (пункт 8
диагноза: объявлены, но не реализованы и не используются). `darray` получил
`Clear()`, `RemoveAt()` и `Data()` — они понадобились кэшу ресурсов и списку
отрисовки. Шейдер `solid_color.vs` теперь действительно читает
`u_Projection * u_View * u_Model`, иначе встроенные uniform'ы остались бы
вырезанными линкером и непроверяемыми.
