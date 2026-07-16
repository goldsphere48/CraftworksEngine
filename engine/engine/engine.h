#pragma once

#include "platform/platform.h"
#include "graphics/renderer.h"

namespace cw::engine
{
    struct Engine
    {
        cw::platform::Context* Platform = nullptr;
        cw::graphics::Context* Graphics = nullptr;
        bool IsRunning = true;
    };
}
