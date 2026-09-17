#include "assets_manager.h"
#include "logger/log.h"
#include "render/material.h"
#include "render/render.h"
#include "resources_private.h"

namespace cw::assets
{
    // A material holds a reference on its pipeline, so tearing one down gives that
    // reference back. The material cache is therefore always destroyed first.
    static void DestroyMaterialResource(void* userContext, void* resource)
    {
        AssetContext*     assets   = static_cast<AssetContext*>(userContext);
        render::Material* material = static_cast<render::Material*>(resource);

        ReleasePipeline(assets, material->Pipeline);
        render::DestroyMaterial(material);
    }

    ResourceCache* CreateMaterialCache(AssetContext* assets)
    {
        return CreateResourceCache(assets, DestroyMaterialResource);
    }

    render::Material* AcquireMaterial(AssetContext* assets, const char* path)
    {
        if (assets == nullptr || path == nullptr)
        {
            return nullptr;
        }

        if (void* cached = CacheFind(assets->Materials, path))
        {
            return static_cast<render::Material*>(cached);
        }

        MaterialAsset* asset = LoadMaterialAsset(path);
        if (asset == nullptr)
        {
            return nullptr;
        }

        render::Pipeline* pipeline = AcquirePipeline(assets, asset->PipelineAssetPath);
        if (pipeline == nullptr)
        {
            FreeMaterialAsset(asset);
            return nullptr;
        }

        render::MaterialParamDesc* params = new render::MaterialParamDesc[asset->ParametersCount];
        for (usize i = 0; i < asset->ParametersCount; ++i)
        {
            const MaterialAsset::Parameter& parameter = asset->Parameters[i];
            params[i] = {parameter.Name, parameter.Value, parameter.ValueCount};
        }

        render::Material* material = render::CreateMaterial(
            assets->Render,
            pipeline,
            params,
            asset->ParametersCount,
            path
        );

        delete[] params;
        FreeMaterialAsset(asset);

        if (material == nullptr)
        {
            ReleasePipeline(assets, pipeline);
            return nullptr;
        }

        if (!CacheInsert(assets->Materials, path, material))
        {
            render::DestroyMaterial(material);
            ReleasePipeline(assets, pipeline);
            return nullptr;
        }

        return material;
    }

    void ReleaseMaterial(AssetContext* assets, const render::Material* material)
    {
        if (assets == nullptr || material == nullptr)
        {
            return;
        }

        CacheRelease(assets->Materials, const_cast<render::Material*>(material));
    }
}
