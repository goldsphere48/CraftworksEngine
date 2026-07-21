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

    bool GetExeDir(char* out_utf8, size_t size)
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

        int written = WideCharToMultiByte(
            CP_UTF8,
            0,
            buffer,
            -1,
            out_utf8,
            (int)size,
            nullptr,
            nullptr
        );

        return written > 0;
    }

    bool ReadFileToBuffer(const char* utf8_path, void** out_data, size_t* out_size)
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

        void* data = malloc((size_t)size.QuadPart + 1);
        DWORD read = 0;
        BOOL  ok   = ReadFile(file, data, (DWORD)size.QuadPart, &read, nullptr);
        CloseHandle(file);
        if (!ok || read != (DWORD)size.QuadPart)
        {
            free(data);
            return false;
        }

        ((char*)data)[read] = '\0';
        *out_data = data;
        *out_size = (size_t)size.QuadPart;

        return true;
    }
}

#endif
