#pragma once

#include "resource_cache.h"
#include "resources.h"

namespace cw::assets
{
    struct AssetContext
    {
        render::RenderContext* Render    = nullptr;
        ResourceCache*         Pipelines = nullptr;
        ResourceCache*         Materials = nullptr;
    };
}
