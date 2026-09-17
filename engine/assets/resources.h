#pragma once

namespace cw::render
{
    struct RenderContext;
    struct Pipeline;
    struct Material;
}

// Turns paths into render resources. This is the only layer that knows both what
// a file looks like and what the renderer wants; render/ sees neither.

namespace cw::assets
{
    struct AssetContext;

    AssetContext* CreateAssetContext(render::RenderContext* render);

    void DestroyAssetContext(AssetContext* assets);

    render::Pipeline* AcquirePipeline(AssetContext* assets, const char* path);

    void ReleasePipeline(AssetContext* assets, const render::Pipeline* pipeline);

    render::Material* AcquireMaterial(AssetContext* assets, const char* path);

    void ReleaseMaterial(AssetContext* assets, const render::Material* material);
}
