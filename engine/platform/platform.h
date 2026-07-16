#pragma once

namespace cw::platform
{
    struct PlatformContext;

    typedef void (*FWindowCloseCallback)(void* userData);

    struct PlatformParams
    {
        FWindowCloseCallback WindowCloseCallback;
        void*                WindowCloseCallbackUserData;

        int         WindowWidth;
        int         WindowHeight;
        const char* WindowTitle;
    };

    PlatformContext* Create(const PlatformParams* params);

    void Destroy(PlatformContext* ctx);

    void* GetNativeWindowHandle(PlatformContext* ctx);

    void PollEvents();
}
