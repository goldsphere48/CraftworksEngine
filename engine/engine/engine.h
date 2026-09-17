#pragma once

#include "assets/resources.h"
#include "graphics/graphics.h"
#include "platform/platform.h"
#include "render/render.h"

namespace cw::engine
{
    struct Engine
    {
        cw::platform::PlatformContext* Platform  = nullptr;
        cw::graphics::GraphicsContext* Graphics  = nullptr;
        cw::render::RenderContext*     Render    = nullptr;
        cw::assets::AssetContext*      Assets    = nullptr;
        bool                           IsRunning = true;
    };
}

void CW_AppInitialize(const cw::engine::Engine* engine);
void CW_AppUpdate(const cw::engine::Engine* engine);
void CW_AppDestroy(const cw::engine::Engine* engine);
