#ifdef CW_PLATFORM_WINDOWS

#include "platform.h"

#include <windows.h>

namespace cw::platform
{
    struct PlatformContext
    {
        FWindowCloseCallback WindowCloseCallback;
        void*                WindowCloseCallbackUserData;

        HWND      Window;
        HINSTANCE Instance;
    };

    static LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        LONG_PTR lpUser      = GetWindowLongPtr(window, GWLP_USERDATA);
        PlatformContext* ctx = reinterpret_cast<PlatformContext*>(lpUser);

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
                ctx->WindowCloseCallback(ctx->WindowCloseCallbackUserData);
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
        PlatformContext* ctx             = new PlatformContext;
        ctx->WindowCloseCallback         = params->WindowCloseCallback;
        ctx->WindowCloseCallbackUserData = params->WindowCloseCallbackUserData;
        ctx->Instance                    = GetModuleHandleA(nullptr);

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
}

#endif
