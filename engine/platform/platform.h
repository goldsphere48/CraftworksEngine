#pragma once

#include "core/types.h"
#include "input/keycodes.h"

constexpr unsigned int CW_MAX_PATH = 1024;

namespace cw::platform
{
    struct PlatformContext;

    enum EVENT_TYPE
    {
        EVENT_NONE = 0,
        EVENT_WINDOW_CLOSE,
        EVENT_WINDOW_FOCUS,
        EVENT_WINDOW_ACTIVATE,
        EVENT_WINDOW_DEACTIVATE,
        EVENT_KEY_DOWN,
        EVENT_KEY_UP,
        EVENT_MOUSE_BUTTON_DOWN,
        EVENT_MOUSE_BUTTON_UP,
        EVENT_MOUSE_MOVE,
        EVENT_MOUSE_WHEEL,
        EVENT_MOUSE_ENTER,
        EVENT_MOUSE_LEAVE,
    };

    struct Event
    {
        EVENT_TYPE Type;

        union
        {
            struct
            {
                input::KEY     Code;
                input::KEY_MOD Mods;
                bool           IsRepeated;
            } Key;

            struct
            {
                input::MOUSE_BUTTON Button;
                input::KEY_MOD      Mods;
                int                 X;
                int                 Y;
                int                 Wheel;
            } Mouse;

            struct
            {
                bool IsFocused;
            } Window;
        };
    };

    typedef void (*FEventCallback)(const Event* event, void* userData);

    struct PlatformParams
    {
        FEventCallback EventCallback;
        void*          EventCallbackUserData;
        int            WindowWidth;
        int            WindowHeight;
        const char*    WindowTitle;
    };

    PlatformContext* Create(const PlatformParams* params);

    void Destroy(PlatformContext* ctx);

    void* GetNativeWindowHandle(PlatformContext* ctx);

    void PollEvents();

    bool GetExeDir(char* out_utf8, usize size);

    bool ReadFileToBuffer(const char* utf8_path, void** out_data, usize* out_size);
}
