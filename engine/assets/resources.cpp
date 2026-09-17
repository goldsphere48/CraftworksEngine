#include "resources_private.h"

namespace cw::assets
{
    ResourceCache* CreatePipelineCache(AssetContext* assets);
    ResourceCache* CreateMaterialCache(AssetContext* assets);

    AssetContext* CreateAssetContext(render::RenderContext* render)
    {
        if (render == nullptr)
        {
            return nullptr;
        }

        AssetContext* assets = new AssetContext{};
        assets->Render       = render;
        assets->Pipelines    = CreatePipelineCache(assets);
        assets->Materials    = CreateMaterialCache(assets);

        return assets;
    }

    void DestroyAssetContext(AssetContext* assets)
    {
        if (assets == nullptr)
        {
            return;
        }

        // Materials first: each one hands a pipeline reference back on the way out.
        DestroyResourceCache(assets->Materials);
        DestroyResourceCache(assets->Pipelines);

        delete assets;
    }
}
