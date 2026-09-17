#include "assets_manager.h"
#include "core/darray.h"
#include "logger/log.h"
#include "render/pipeline.h"
#include "render/render.h"
#include "resources_private.h"

namespace cw::assets
{
    static void DestroyPipelineResource(void* userContext, void* resource)
    {
        AssetContext* assets = static_cast<AssetContext*>(userContext);
        render::DestroyPipeline(assets->Render, static_cast<render::Pipeline*>(resource));
    }

    ResourceCache* CreatePipelineCache(AssetContext* assets)
    {
        return CreateResourceCache(assets, DestroyPipelineResource);
    }

    render::Pipeline* AcquirePipeline(AssetContext* assets, const char* path)
    {
        if (assets == nullptr || path == nullptr)
        {
            return nullptr;
        }

        if (void* cached = CacheFind(assets->Pipelines, path))
        {
            return static_cast<render::Pipeline*>(cached);
        }

        PipelineAsset* asset = LoadPipelineAsset(path);
        if (asset == nullptr)
        {
            return nullptr;
        }

        graphics::VertexAttribute* attributes =
            new graphics::VertexAttribute[asset->VerticesAttributesCount];
        for (usize i = 0; i < asset->VerticesAttributesCount; ++i)
        {
            const PipelineAsset::VertexParameter& attribute = asset->VerticesAttributesInfo[i];
            attributes[i] = {attribute.Name, attribute.Format};
        }

        render::PipelineUniformDesc* uniforms =
            new render::PipelineUniformDesc[asset->UniformsCount];
        for (usize i = 0; i < asset->UniformsCount; ++i)
        {
            const PipelineAsset::UniformParameter& uniform = asset->UniformsInfo[i];
            uniforms[i] = {uniform.Name, uniform.Type};
        }

        const render::PipelineCreateDesc desc = {
            .DebugName      = path,
            .VertexSource   = static_cast<const char*>(asset->VertexBuffer->Data),
            .FragmentSource = static_cast<const char*>(asset->FragmentBuffer->Data),
            .Attributes     = attributes,
            .AttributeCount = asset->VerticesAttributesCount,
            .Uniforms       = uniforms,
            .UniformCount   = asset->UniformsCount,
        };

        render::Pipeline* pipeline = render::CreatePipeline(assets->Render, &desc);

        delete[] uniforms;
        delete[] attributes;
        FreePipelineAsset(asset);

        if (pipeline == nullptr)
        {
            return nullptr;
        }

        if (!CacheInsert(assets->Pipelines, path, pipeline))
        {
            render::DestroyPipeline(assets->Render, pipeline);
            return nullptr;
        }

        return pipeline;
    }

    void ReleasePipeline(AssetContext* assets, const render::Pipeline* pipeline)
    {
        if (assets == nullptr || pipeline == nullptr)
        {
            return;
        }

        CacheRelease(assets->Pipelines, const_cast<render::Pipeline*>(pipeline));
    }
}
