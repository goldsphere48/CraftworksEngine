#ifdef CW_PLATFORM_WINDOWS

#include "input/keycodes.h"
#include "platform.h"
#include "debug/debug.h"

#include <windows.h>
#include <windowsx.h>
#include <cstdio>

namespace cw::platform
{
    struct PlatformContext
    {
        FEventCallback EventCallback;
        void*          EventCallbackUserData;
        HWND           Window;
        HINSTANCE      Instance;
        bool           MouseTracked;
        uint8          MouseButtons;
    };

    static uint16 g_KeyTable[256];

    static bool g_KeyTableInitialized = false;

    static void InitializeKeyTable()
    {
        if (g_KeyTableInitialized)
        {
            return;
        }

        g_KeyTableInitialized = true;

        for (int i = 0; i < 26; ++i)
        {
            g_KeyTable['A' + i] = (uint16)(input::KEY_A + i);
        }

        for (int i = 0; i < 10; ++i)
        {
            g_KeyTable['0' + i] = (uint16)(input::KEY_0 + i);
        }

        for (int i = 0; i < 10; ++i)
        {
            g_KeyTable[VK_NUMPAD0 + i] = (uint16)(input::KEY_KP_0 + i);
        }

        for (int i = 0; i < 12; ++i)
        {
            g_KeyTable[VK_F1 + i] = (uint16)(input::KEY_F1 + i);
        }

        g_KeyTable[VK_SPACE]      = input::KEY_SPACE;
        g_KeyTable[VK_OEM_7]      = input::KEY_APOSTROPHE;
        g_KeyTable[VK_OEM_COMMA]  = input::KEY_COMMA;
        g_KeyTable[VK_OEM_MINUS]  = input::KEY_MINUS;
        g_KeyTable[VK_OEM_PERIOD] = input::KEY_PERIOD;
        g_KeyTable[VK_OEM_2]      = input::KEY_SLASH;
        g_KeyTable[VK_OEM_1]      = input::KEY_SEMICOLON;
        g_KeyTable[VK_OEM_PLUS]   = input::KEY_EQUAL;
        g_KeyTable[VK_OEM_4]      = input::KEY_LEFT_BRACKET;
        g_KeyTable[VK_OEM_5]      = input::KEY_BACKSLASH;
        g_KeyTable[VK_OEM_6]      = input::KEY_RIGHT_BRACKET;
        g_KeyTable[VK_OEM_3]      = input::KEY_GRAVE;

        g_KeyTable[VK_ESCAPE]   = input::KEY_ESCAPE;
        g_KeyTable[VK_RETURN]   = input::KEY_ENTER;
        g_KeyTable[VK_TAB]      = input::KEY_TAB;
        g_KeyTable[VK_BACK]     = input::KEY_BACKSPACE;
        g_KeyTable[VK_INSERT]   = input::KEY_INSERT;
        g_KeyTable[VK_DELETE]   = input::KEY_DELETE;
        g_KeyTable[VK_RIGHT]    = input::KEY_RIGHT;
        g_KeyTable[VK_LEFT]     = input::KEY_LEFT;
        g_KeyTable[VK_DOWN]     = input::KEY_DOWN;
        g_KeyTable[VK_UP]       = input::KEY_UP;
        g_KeyTable[VK_PRIOR]    = input::KEY_PAGE_UP;
        g_KeyTable[VK_NEXT]     = input::KEY_PAGE_DOWN;
        g_KeyTable[VK_HOME]     = input::KEY_HOME;
        g_KeyTable[VK_END]      = input::KEY_END;
        g_KeyTable[VK_CAPITAL]  = input::KEY_CAPS_LOCK;
        g_KeyTable[VK_SCROLL]   = input::KEY_SCROLL_LOCK;
        g_KeyTable[VK_NUMLOCK]  = input::KEY_NUM_LOCK;
        g_KeyTable[VK_SNAPSHOT] = input::KEY_PRINT_SCREEN;
        g_KeyTable[VK_PAUSE]    = input::KEY_PAUSE;

        g_KeyTable[VK_DECIMAL]  = input::KEY_KP_DECIMAL;
        g_KeyTable[VK_DIVIDE]   = input::KEY_KP_DIVIDE;
        g_KeyTable[VK_MULTIPLY] = input::KEY_KP_MULTIPLY;
        g_KeyTable[VK_SUBTRACT] = input::KEY_KP_SUBTRACT;
        g_KeyTable[VK_ADD]      = input::KEY_KP_ADD;

        g_KeyTable[VK_LSHIFT]   = input::KEY_LEFT_SHIFT;
        g_KeyTable[VK_LCONTROL] = input::KEY_LEFT_CONTROL;
        g_KeyTable[VK_LMENU]    = input::KEY_LEFT_ALT;
        g_KeyTable[VK_LWIN]     = input::KEY_LEFT_SUPER;
        g_KeyTable[VK_RSHIFT]   = input::KEY_RIGHT_SHIFT;
        g_KeyTable[VK_RCONTROL] = input::KEY_RIGHT_CONTROL;
        g_KeyTable[VK_RMENU]    = input::KEY_RIGHT_ALT;
        g_KeyTable[VK_RWIN]     = input::KEY_RIGHT_SUPER;
        g_KeyTable[VK_APPS]     = input::KEY_MENU;

        g_KeyTable[VK_SHIFT]   = input::KEY_LEFT_SHIFT;
        g_KeyTable[VK_CONTROL] = input::KEY_LEFT_CONTROL;
        g_KeyTable[VK_MENU]    = input::KEY_LEFT_ALT;
    }

    static input::KEY TranslateKey(WPARAM wParam, LPARAM lParam)
    {
        UINT scancode = (UINT)((lParam >> 16) & 0xFF);
        bool extended = (UINT)((lParam >> 24) & 1) > 0;

        if (wParam == VK_SHIFT)
        {
            UINT vk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
            if (vk != 0)
            {
                wParam = vk;
            }
        }
        else if (wParam == VK_CONTROL)
        {
            wParam = extended ? VK_RCONTROL : VK_LCONTROL;
        }
        else if (wParam == VK_MENU)
        {
            wParam = extended ? VK_RMENU : VK_LMENU;
        }
        else if (wParam == VK_RETURN && extended)
        {
            return input::KEY_KP_ENTER;
        }

        if (wParam >= 256)
        {
            return input::KEY_UNKNOWN;
        }

        return (input::KEY)g_KeyTable[wParam];
    }

    static input::KEY_MOD GetKeyMods()
    {
        uint8 mods = 0;

        if ((GetKeyState(VK_SHIFT) & 0x8000) != 0)
        {
            mods |= (uint8)input::KEY_MOD_SHIFT;
        }

        if ((GetKeyState(VK_CONTROL) & 0x8000) != 0)
        {
            mods |= (uint8)input::KEY_MOD_CONTROL;
        }

        if ((GetKeyState(VK_MENU) & 0x8000) != 0)
        {
            mods |= (uint8)input::KEY_MOD_ALT;
        }

        if ((GetKeyState(VK_CAPITAL) & 1) != 0)
        {
            mods |= (uint8)input::KEY_MOD_CAPS_LOCK;
        }

        if ((GetKeyState(VK_NUMLOCK) & 1) != 0)
        {
            mods |= (uint8)input::KEY_MOD_NUM_LOCK;
        }

        if ((GetKeyState(VK_LWIN) & 0x8000) != 0 || (GetKeyState(VK_RWIN) & 0x8000) != 0)
        {
            mods |= (uint8)input::KEY_MOD_SUPER;
        }

        return (input::KEY_MOD)mods;
    }

    static void HandleKeyEvent(
        const PlatformContext* ctx,
        WPARAM wParam,
        LPARAM lParam,
        EVENT_TYPE type
    )
    {
        input::KEY key = TranslateKey(wParam, lParam);
        if (key == input::KEY_UNKNOWN)
        {
            return;
        }

        Event event    = {};
        event.Type     = type;
        event.Key.Code = key;
        event.Key.Mods = GetKeyMods();
        if (type == EVENT_KEY_DOWN)
        {
            event.Key.IsRepeated = ((lParam >> 30) & 1) > 0;
        }

        ctx->EventCallback(&event, ctx->EventCallbackUserData);
    }

    static void HandleMouseButtonEvent(
        PlatformContext*    ctx,
        input::MOUSE_BUTTON button,
        EVENT_TYPE          type,
        LPARAM              lParam
    )
    {
        uint8 mask = (uint8)(1 << button);

        if (type == EVENT_MOUSE_BUTTON_DOWN)
        {
            if (ctx->MouseButtons == 0)
            {
                SetCapture(ctx->Window);
            }

            ctx->MouseButtons |= mask;
        }
        else
        {
            ctx->MouseButtons = (uint8)(ctx->MouseButtons & ~mask);

            if (ctx->MouseButtons == 0)
            {
                ReleaseCapture();
            }
        }

        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        Event event        = {};
        event.Mouse.X      = xPos;
        event.Mouse.Y      = yPos;
        event.Mouse.Button = button;
        event.Type         = type;
        event.Mouse.Mods   = GetKeyMods();
        ctx->EventCallback(&event, ctx->EventCallbackUserData);
    }

    static void ReleaseAllMouseButtons(PlatformContext* ctx)
    {
        if (ctx->MouseButtons == 0)
        {
            return;
        }

        POINT cursor = {};
        GetCursorPos(&cursor);
        ScreenToClient(ctx->Window, &cursor);

        for (uint8 i = 0; i < input::MOUSE_BUTTON_COUNT; ++i)
        {
            if ((ctx->MouseButtons & (1 << i)) == 0)
            {
                continue;
            }

            Event event        = {};
            event.Type         = EVENT_MOUSE_BUTTON_UP;
            event.Mouse.Mods   = GetKeyMods();
            event.Mouse.Button = (input::MOUSE_BUTTON)i;
            event.Mouse.X      = cursor.x;
            event.Mouse.Y      = cursor.y;

            ctx->EventCallback(&event, ctx->EventCallbackUserData);
        }

        ctx->MouseButtons = 0;
    }

    static LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        LONG_PTR         lpUser = GetWindowLongPtr(window, GWLP_USERDATA);
        PlatformContext* ctx    = reinterpret_cast<PlatformContext*>(lpUser);

        if (ctx == nullptr && msg != WM_NCCREATE)
        {
            return DefWindowProc(window, msg, wParam, lParam);
        }

        switch (msg)
        {
            case WM_NCCREATE:
            {
                CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
                ctx                   = static_cast<PlatformContext*>(pCreate->lpCreateParams);
                SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
                return DefWindowProc(window, msg, wParam, lParam);
            }
            case WM_CLOSE:
            {
                Event event = {};
                event.Type  = EVENT_WINDOW_CLOSE;
                ctx->EventCallback(&event, ctx->EventCallbackUserData);
                break;
            }
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            {
                HandleKeyEvent(ctx, wParam, lParam, EVENT_KEY_DOWN);
                if (msg == WM_SYSKEYDOWN)
                {
                    return DefWindowProc(window, msg, wParam, lParam);
                }
                break;
            }
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                HandleKeyEvent(ctx, wParam, lParam, EVENT_KEY_UP);
                if (msg == WM_SYSKEYUP)
                {
                    return DefWindowProc(window, msg, wParam, lParam);
                }
                break;
            }
            case WM_LBUTTONDOWN:
            {
                HandleMouseButtonEvent(
                    ctx,
                    input::MOUSE_BUTTON_LEFT,
                    EVENT_MOUSE_BUTTON_DOWN,
                    lParam
                );
                break;
            }
            case WM_RBUTTONDOWN:
            {
                HandleMouseButtonEvent(
                    ctx,
                    input::MOUSE_BUTTON_RIGHT,
                    EVENT_MOUSE_BUTTON_DOWN,
                    lParam
                );
                break;
            }
            case WM_MBUTTONDOWN:
            {
                HandleMouseButtonEvent(
                    ctx,
                    input::MOUSE_BUTTON_MIDDLE,
                    EVENT_MOUSE_BUTTON_DOWN,
                    lParam
                );
                break;
            }
            case WM_XBUTTONDOWN:
            {
                input::MOUSE_BUTTON button;
                button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? input::MOUSE_BUTTON_X1
                                                                : input::MOUSE_BUTTON_X2;
                HandleMouseButtonEvent(ctx, button, EVENT_MOUSE_BUTTON_DOWN, lParam);
                break;
            }
            case WM_LBUTTONUP:
            {
                HandleMouseButtonEvent(
                    ctx,
                    input::MOUSE_BUTTON_LEFT,
                    EVENT_MOUSE_BUTTON_UP,
                    lParam
                );
                break;
            }
            case WM_RBUTTONUP:
            {
                HandleMouseButtonEvent(
                    ctx,
                    input::MOUSE_BUTTON_RIGHT,
                    EVENT_MOUSE_BUTTON_UP,
                    lParam
                );
                break;
            }
            case WM_MBUTTONUP:
            {
                HandleMouseButtonEvent(
                    ctx,
                    input::MOUSE_BUTTON_MIDDLE,
                    EVENT_MOUSE_BUTTON_UP,
                    lParam
                );
                break;
            }
            case WM_XBUTTONUP:
            {
                input::MOUSE_BUTTON button;
                button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? input::MOUSE_BUTTON_X1
                                                                : input::MOUSE_BUTTON_X2;

                HandleMouseButtonEvent(ctx, button, EVENT_MOUSE_BUTTON_UP, lParam);
                break;
            }
            case WM_MOUSEMOVE:
            {
                int xPos = GET_X_LPARAM(lParam);
                int yPos = GET_Y_LPARAM(lParam);

                if (!ctx->MouseTracked)
                {
                    RECT  client = {};
                    POINT point  = { xPos, yPos };
                    GetClientRect(window, &client);

                    if (PtInRect(&client, point))
                    {
                        TRACKMOUSEEVENT tme = {};
                        tme.cbSize          = sizeof(tme);
                        tme.dwFlags         = TME_LEAVE;
                        tme.hwndTrack       = window;
                        TrackMouseEvent(&tme);

                        ctx->MouseTracked = true;

                        Event enter   = {};
                        enter.Type    = EVENT_MOUSE_ENTER;
                        enter.Mouse.X = xPos;
                        enter.Mouse.Y = yPos;

                        ctx->EventCallback(&enter, ctx->EventCallbackUserData);
                    }
                }

                Event event   = {};
                event.Type    = EVENT_MOUSE_MOVE;
                event.Mouse.X = xPos;
                event.Mouse.Y = yPos;

                ctx->EventCallback(&event, ctx->EventCallbackUserData);
                break;
            }
            case WM_MOUSEWHEEL:
            {
                int   zDelta      = GET_WHEEL_DELTA_WPARAM(wParam);
                Event event       = {};
                event.Type        = EVENT_MOUSE_WHEEL;
                event.Mouse.Wheel = zDelta > 0 ? 1 : -1;

                ctx->EventCallback(&event, ctx->EventCallbackUserData);
                break;
            }
            case WM_KILLFOCUS:
            case WM_SETFOCUS:
            {
                Event event            = {};
                event.Type             = EVENT_WINDOW_FOCUS;
                event.Window.IsFocused = msg == WM_SETFOCUS;

                ctx->EventCallback(&event, ctx->EventCallbackUserData);
                break;
            }
            case WM_ACTIVATE:
            {
                Event event = {};
                event.Type  = LOWORD(wParam) == WA_INACTIVE ? EVENT_WINDOW_DEACTIVATE
                                                            : EVENT_WINDOW_ACTIVATE;


                ctx->EventCallback(&event, ctx->EventCallbackUserData);
                break;
            }
            case WM_CAPTURECHANGED:
            {
                ReleaseAllMouseButtons(ctx);
                break;
            }
            case WM_MOUSELEAVE:
            {
                ctx->MouseTracked = false;

                Event event = {};
                event.Type  = EVENT_MOUSE_LEAVE;

                ctx->EventCallback(&event, ctx->EventCallbackUserData);
                break;
            }
            default:
            {
                return DefWindowProc(window, msg, wParam, lParam);
            }
        }
        return 0;
    }

    PlatformContext* Create(const PlatformParams* params)
    {
        PlatformContext* ctx       = new PlatformContext;
        ctx->EventCallback         = params->EventCallback;
        ctx->EventCallbackUserData = params->EventCallbackUserData;
        ctx->Instance              = GetModuleHandleA(nullptr);
        ctx->MouseTracked          = false;
        ctx->MouseButtons          = 0;

        InitializeKeyTable();

        WNDCLASSEXA wc   = {};
        wc.cbSize        = sizeof(WNDCLASSEXA);
        wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc   = WindowProc;
        wc.hInstance     = ctx->Instance;
        wc.lpszClassName = "CW_WINDOW_CLASS";

        RegisterClassEx(&wc);

        ctx->Window = CreateWindowEx(
            0,
            wc.lpszClassName,
            params->WindowTitle,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            params->WindowWidth,
            params->WindowHeight,
            nullptr,
            nullptr,
            wc.hInstance,
            ctx
        );

        if (ctx->Window == nullptr)
        {
            Destroy(ctx);
            return nullptr;
        }

        ShowWindow(ctx->Window, SW_SHOW);

        return ctx;
    }

    void Destroy(PlatformContext* ctx)
    {
        if (ctx->Window != nullptr)
        {
            SetWindowLongPtr(ctx->Window, GWLP_USERDATA, 0);
            DestroyWindow(ctx->Window);
            ctx->Window = nullptr;
        }

        delete ctx;
    }

    void* GetNativeWindowHandle(PlatformContext* ctx)
    {
        return ctx->Window;
    }

    void PollEvents()
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    bool GetExeDir(char* out_utf8, usize size)
    {
        wchar_t buffer[CW_MAX_PATH];

        DWORD len = GetModuleFileNameW(nullptr, buffer, CW_MAX_PATH);
        if (len == 0 || len == CW_MAX_PATH)
        {
            return false;
        }

        for (DWORD i = len; i > 0; --i)
        {
            if (buffer[i - 1] == L'\\' || buffer[i - 1] == L'/')
            {
                buffer[i] = 0;
                break;
            }
        }

        int written =
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, out_utf8, (int)size, nullptr, nullptr);

        return written > 0;
    }

    bool ReadFileToBuffer(const char* utf8_path, void** out_data, usize* out_size)
    {
        wchar_t path[CW_MAX_PATH];
        if (MultiByteToWideChar(CP_UTF8, 0, utf8_path, -1, path, CW_MAX_PATH) == 0)
        {
            return false;
        }

        HANDLE file = CreateFileW(
            path,
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (file == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        LARGE_INTEGER size;
        if (!GetFileSizeEx(file, &size))
        {
            CloseHandle(file);
            return false;
        }

        void* data = malloc((usize)size.QuadPart + 1);
        DWORD read = 0;
        BOOL  ok   = ReadFile(file, data, (DWORD)size.QuadPart, &read, nullptr);
        CloseHandle(file);
        if (!ok || read != (DWORD)size.QuadPart)
        {
            free(data);
            return false;
        }

        ((char*)data)[read] = '\0';
        *out_data           = data;
        *out_size           = (usize)size.QuadPart;

        return true;
    }
}

#endif
