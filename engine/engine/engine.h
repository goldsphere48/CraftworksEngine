#pragma once

#include "platform/platform.h"
#include "graphics/renderer.h"

namespace cw::engine
{
    struct Engine
    {
        cw::platform::PlatformContext* Platform = nullptr;
        cw::graphics::GraphicsContext* Graphics = nullptr;
        bool IsRunning = true;
    };
}

void CW_AppInitialize(const cw::engine::Engine* engine);
void CW_AppUpdate(const cw::engine::Engine* engine);
void CW_AppDestroy(const cw::engine::Engine* engine);
