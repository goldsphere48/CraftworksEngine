#include "input.h"

#include "platform/platform.h"

#include <memory.h>

namespace cw::input
{
    enum KeyState
    {
        STATE_UP = 0,
        STATE_DOWN,
        STATE_PRESSED,
        STATE_RELEASED,
    };

    static struct
    {
        KeyState KeyStates[KEY_COUNT];
        KeyState MouseButtonStates[MOUSE_BUTTON_COUNT];
        KEY_MOD  Mods       = KEY_MOD_NONE;
        int      MouseX     = 0;
        int      MouseY     = 0;
        int      PrevMouseX = 0;
        int      PrevMouseY = 0;
        int      MouseWheel = 0;
    } g_InputState;

    static void ClearKeyStates()
    {
        memset(g_InputState.KeyStates, 0, sizeof(g_InputState.KeyStates));
        memset(g_InputState.MouseButtonStates, 0, sizeof(g_InputState.MouseButtonStates));
        g_InputState.MouseWheel = 0;
        g_InputState.Mods       = KEY_MOD_NONE;
    }

    void Initialize()
    {
        memset(&g_InputState, 0, sizeof(g_InputState));
    }

    void EndFrame()
    {
        for (int i = 0; i < KEY_COUNT; ++i)
        {
            KeyState& state = g_InputState.KeyStates[i];
            if (state == STATE_PRESSED)
            {
                state = STATE_DOWN;
            }
            else if (state == STATE_RELEASED)
            {
                state = STATE_UP;
            }
        }

        for (int i = 0; i < MOUSE_BUTTON_COUNT; ++i)
        {
            KeyState& state = g_InputState.MouseButtonStates[i];
            if (state == STATE_PRESSED)
            {
                state = STATE_DOWN;
            }
            else if (state == STATE_RELEASED)
            {
                state = STATE_UP;
            }
        }

        g_InputState.MouseWheel = 0;
        g_InputState.PrevMouseX = g_InputState.MouseX;
        g_InputState.PrevMouseY = g_InputState.MouseY;
    }

    void HandleEvent(const platform::Event* event)
    {
        if (event->Type == platform::EVENT_KEY_DOWN)
        {
            KeyState& state = g_InputState.KeyStates[event->Key.Code];
            if (state != STATE_DOWN && state != STATE_PRESSED)
            {
                state = STATE_PRESSED;
            }
            g_InputState.Mods = event->Key.Mods;
            return;
        }

        if (event->Type == platform::EVENT_KEY_UP)
        {
            KeyState& state = g_InputState.KeyStates[event->Key.Code];
            if (state == STATE_DOWN || state == STATE_PRESSED)
            {
                state = STATE_RELEASED;
            }
            g_InputState.Mods = event->Key.Mods;
            return;
        }

        if (event->Type == platform::EVENT_MOUSE_BUTTON_DOWN)
        {
            KeyState& state = g_InputState.MouseButtonStates[event->Mouse.Button];
            if (state != STATE_DOWN && state != STATE_PRESSED)
            {
                state = STATE_PRESSED;
            }
            g_InputState.MouseX = event->Mouse.X;
            g_InputState.MouseY = event->Mouse.Y;
            g_InputState.Mods   = event->Mouse.Mods;
            return;
        }

        if (event->Type == platform::EVENT_MOUSE_BUTTON_UP)
        {
            KeyState& state = g_InputState.MouseButtonStates[event->Mouse.Button];
            if (state == STATE_DOWN || state == STATE_PRESSED)
            {
                state = STATE_RELEASED;
            }
            g_InputState.MouseX = event->Mouse.X;
            g_InputState.MouseY = event->Mouse.Y;
            g_InputState.Mods   = event->Mouse.Mods;
            return;
        }

        if (event->Type == platform::EVENT_MOUSE_ENTER)
        {
            g_InputState.MouseX     = event->Mouse.X;
            g_InputState.MouseY     = event->Mouse.Y;
            g_InputState.PrevMouseX = event->Mouse.X;
            g_InputState.PrevMouseY = event->Mouse.Y;
            return;
        }

        if (event->Type == platform::EVENT_MOUSE_MOVE)
        {
            g_InputState.MouseX = event->Mouse.X;
            g_InputState.MouseY = event->Mouse.Y;
            return;
        }

        if (event->Type == platform::EVENT_MOUSE_WHEEL)
        {
            g_InputState.MouseWheel += event->Mouse.Wheel;
            return;
        }

        if (event->Type == platform::EVENT_WINDOW_FOCUS && !event->Window.IsFocused)
        {
            ClearKeyStates();
            return;
        }
    }

    bool IsKeyDown(KEY key)
    {
        KeyState state = g_InputState.KeyStates[key];
        return state == STATE_DOWN || state == STATE_PRESSED;
    }

    bool IsKeyReleased(KEY key)
    {
        return g_InputState.KeyStates[key] == STATE_RELEASED;
    }

    bool IsKeyPressed(KEY key)
    {
        return g_InputState.KeyStates[key] == STATE_PRESSED;
    }

    bool IsMouseButtonDown(MOUSE_BUTTON button)
    {
        KeyState state = g_InputState.MouseButtonStates[button];
        return state == STATE_DOWN || state == STATE_PRESSED;
    }

    bool IsMouseButtonReleased(MOUSE_BUTTON button)
    {
        return g_InputState.MouseButtonStates[button] == STATE_RELEASED;
    }

    bool IsMouseButtonPressed(MOUSE_BUTTON button)
    {
        return g_InputState.MouseButtonStates[button] == STATE_PRESSED;
    }

    int GetMouseX()
    {
        return g_InputState.MouseX;
    }

    int GetMouseY()
    {
        return g_InputState.MouseY;
    }

    int GetMouseWheel()
    {
        return g_InputState.MouseWheel;
    }

    math::Vec2i GetMousePosition()
    {
        return {g_InputState.MouseX, g_InputState.MouseY};
    }

    math::Vec2i GetMouseDelta()
    {
        return {
            g_InputState.MouseX - g_InputState.PrevMouseX,
            g_InputState.MouseY - g_InputState.PrevMouseY
        };
    }

    KEY_MOD GetMods()
    {
        return g_InputState.Mods;
    }

    bool IsModDown(KEY_MOD mod_flags)
    {
        return mod_flags != KEY_MOD_NONE && (g_InputState.Mods & mod_flags) == mod_flags;
    }
}
