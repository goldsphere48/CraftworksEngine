#pragma once

namespace cw::graphics
{
    enum RENDER_BACKEND_TYPE
    {
        RENDER_BACKEND_NONE,
        RENDER_BACKEND_OPENGL,
    };

    typedef bool (*FInitialize)(void* window);

    typedef void (*FDestroy)();

    typedef void (*FBeginFrame)();

    typedef void (*FEndFrame)();

    struct RenderBackend
    {
        FInitialize Initialize;
        FDestroy    Destroy;
        FBeginFrame BeginFrame;
        FEndFrame   EndFrame;

        RENDER_BACKEND_TYPE BackendType = RENDER_BACKEND_NONE;
    };
}
