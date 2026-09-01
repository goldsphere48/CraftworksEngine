#include "pipeline_manager.h"

#include "assets/assets_manager.h"
#include "core/darray.h"
#include "graphics/renderer.h"
#include "logger/log.h"
#include "utils/hash.h"

#include <cstring>

namespace cw::graphics
{
    struct PipelineEntry
    {
        PipelineResource* Pipeline;
        uint64            Hash;
        char*             Path;
    };

    struct PipelineManager
    {
        darray<PipelineEntry> Entries;
    };

    static char* CopyString(const char* value)
    {
        const usize length = strlen(value);
        char*       copy   = new char[length + 1];
        memcpy(copy, value, length + 1);
        return copy;
    }

    static void DestroyPipelineResource(PipelineResource* resource)
    {
        if (resource == nullptr)
        {
            return;
        }

        if (resource->BackendPipeline != nullptr)
        {
            graphics::DestroyPipeline(resource->BackendPipeline);
        }

        for (usize i = 0; i < resource->UniformsCount; ++i)
        {
            delete[] resource->Uniforms[i].Name;
        }

        delete[] resource->Uniforms;
        delete resource;
    }

    PipelineManager* CreatePipelineManager()
    {
        return new PipelineManager;
    }

    void DestroyPipelineManager(PipelineManager* manager)
    {
        if (manager == nullptr)
        {
            return;
        }

        for (usize i = 0; i < manager->Entries.Count(); ++i)
        {
            PipelineEntry& entry = manager->Entries[i];
            DestroyPipelineResource(entry.Pipeline);
            delete[] entry.Path;
        }

        delete manager;
    }

    static PipelineResource* FindPipeline(
        const PipelineManager* manager,
        const char*            path,
        uint64                 pathHash
    )
    {
        for (usize i = 0; i < manager->Entries.Count(); ++i)
        {
            const PipelineEntry& entry = manager->Entries[i];
            if (entry.Hash == pathHash && strcmp(entry.Path, path) == 0)
            {
                return entry.Pipeline;
            }
        }

        return nullptr;
    }

    static bool ValidateUniformNames(const assets::PipelineAsset* asset, const char* path)
    {
        for (usize i = 0; i < asset->UniformsCount; ++i)
        {
            const char*  name     = asset->UniformsInfo[i].Name;
            const uint64 nameHash = utils::HashString(name);

            for (usize j = 0; j < i; ++j)
            {
                const char* otherName = asset->UniformsInfo[j].Name;
                if (utils::HashString(otherName) != nameHash)
                {
                    continue;
                }

                if (strcmp(otherName, name) == 0)
                {
                    CW_ERROR("%s: uniform \"%s\" is declared more than once", path, name);
                }
                else
                {
                    CW_ERROR(
                        "%s: uniform hash collision between \"%s\" and \"%s\"",
                        path,
                        otherName,
                        name
                    );
                }

                return false;
            }
        }

        return true;
    }

    static HPipeline CreatePipelineFromAsset(const assets::PipelineAsset* asset)
    {
        VertexAttribute* attributes = new VertexAttribute[asset->VerticesAttributesCount];
        for (usize i = 0; i < asset->VerticesAttributesCount; ++i)
        {
            const assets::PipelineAsset::VertexParameter& attribute =
                asset->VerticesAttributesInfo[i];
            attributes[i] = { attribute.Name, attribute.Format };
        }

        UniformDesc* uniforms = new UniformDesc[asset->UniformsCount];
        for (usize i = 0; i < asset->UniformsCount; ++i)
        {
            const assets::PipelineAsset::UniformParameter& uniform = asset->UniformsInfo[i];
            uniforms[i] = { uniform.Type, uniform.Name };
        }

        const PipelineDesc desc = {
            .VertexSource   = static_cast<const char*>(asset->VertexBuffer->Data),
            .FragmentSource = static_cast<const char*>(asset->FragmentBuffer->Data),
            .Attributes     = attributes,
            .AttributeCount = static_cast<int>(asset->VerticesAttributesCount),
            .Uniforms       = uniforms,
            .UniformsCount  = asset->UniformsCount,
        };

        HPipeline pipeline = graphics::CreatePipeline(&desc);

        delete[] uniforms;
        delete[] attributes;

        return pipeline;
    }

    const PipelineResource* AcquirePipeline(PipelineManager* manager, const char* path)
    {
        if (manager == nullptr || path == nullptr)
        {
            return nullptr;
        }

        const uint64 pathHash = utils::HashString(path);
        if (PipelineResource* resource = FindPipeline(manager, path, pathHash); resource != nullptr)
        {
            return resource;
        }

        assets::PipelineAsset* asset = assets::LoadPipelineAsset(path);
        if (asset == nullptr)
        {
            return nullptr;
        }

        if (!ValidateUniformNames(asset, path))
        {
            assets::FreePipelineAsset(asset);
            return nullptr;
        }

        HPipeline backendPipeline = CreatePipelineFromAsset(asset);
        if (backendPipeline == nullptr)
        {
            CW_ERROR("Failed to create pipeline %s", path);
            assets::FreePipelineAsset(asset);
            return nullptr;
        }

        PipelineResource* resource = new PipelineResource{};
        resource->BackendPipeline  = backendPipeline;
        resource->UniformsCount    = asset->UniformsCount;
        resource->Uniforms         = new PipelineUniform[resource->UniformsCount]{};

        for (usize i = 0; i < resource->UniformsCount; ++i)
        {
            const assets::PipelineAsset::UniformParameter& source = asset->UniformsInfo[i];
            PipelineUniform&                               uniform = resource->Uniforms[i];

            uniform.Name     = CopyString(source.Name);
            uniform.NameHash = utils::HashString(source.Name);
            uniform.Type     = source.Type;

            graphics::GetUniform(
                resource->BackendPipeline,
                uniform.NameHash,
                &uniform.BackendUniform
            );

            if (uniform.BackendUniform == nullptr)
            {
                CW_ERROR("%s: backend did not create uniform \"%s\"", path, uniform.Name);
                assets::FreePipelineAsset(asset);
                DestroyPipelineResource(resource);
                return nullptr;
            }
        }

        assets::FreePipelineAsset(asset);

        PipelineEntry entry = {
            .Pipeline = resource,
            .Hash     = pathHash,
            .Path     = CopyString(path),
        };

        if (!manager->Entries.Add(entry))
        {
            CW_ERROR("Failed to cache pipeline %s", path);
            delete[] entry.Path;
            DestroyPipelineResource(resource);
            return nullptr;
        }

        return resource;
    }
}
