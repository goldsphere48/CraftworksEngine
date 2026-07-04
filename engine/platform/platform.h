#pragma once

namespace cw::platform
{
    typedef struct Context* HContext;

    typedef void (*FWindowCloseCallback)(void* userData);

    struct PlatformParams
    {
        FWindowCloseCallback WindowCloseCallback;
        void*                WindowCloseCallbackUserData;

        int         WindowWidth;
        int         WindowHeight;
        const char* WindowTitle;
    };

    Context* CreateContext(const PlatformParams* params);

    void Destroy(Context* ctx);

    void PollEvents();
}
