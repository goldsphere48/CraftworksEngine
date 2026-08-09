#pragma once

#include "keycodes.h"
#include "math/vector.h"

namespace cw::platform
{
    struct Event;
}

namespace cw::input
{
    void Initialize();

    void EndFrame();
    void HandleEvent(const platform::Event* event);
    
    bool IsKeyDown(KEY key);
    bool IsKeyReleased(KEY key);
    bool IsKeyPressed(KEY key);

    bool IsMouseButtonDown(MOUSE_BUTTON button);
    bool IsMouseButtonReleased(MOUSE_BUTTON button);
    bool IsMouseButtonPressed(MOUSE_BUTTON button);

    int GetMouseX();
    int GetMouseY();
    int GetMouseWheel();
    Vec2i GetMousePosition();
    Vec2i GetMouseDelta();

    KEY_MOD GetMods();
    bool IsModDown(KEY_MOD mod_flags);
}
