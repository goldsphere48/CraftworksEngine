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
