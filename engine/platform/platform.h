#pragma once

namespace cw::platform
{
    struct Context;

    typedef void (*FWindowCloseCallback)(void* userData);

    struct ContextParams
    {
        FWindowCloseCallback WindowCloseCallback;
        void*                WindowCloseCallbackUserData;

        int         WindowWidth;
        int         WindowHeight;
        const char* WindowTitle;
    };

    Context* Create(const ContextParams* params);

    void Destroy(Context* ctx);

    void* GetNativeWindowHandle(Context* ctx);

    void PollEvents();
}
