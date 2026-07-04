#pragma once

#include "platform/platform.h"

namespace cw::engine
{
    struct Engine
    {
        cw::platform::HContext Platform;
        bool IsRunning = true;
    };
}
