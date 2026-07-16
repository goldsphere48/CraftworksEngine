#pragma once

namespace cw::graphics
{
    enum RENDER_BACKEND
    {
        RENDER_BACKEND_NONE,
        RENDER_BACKEND_OPENGL,
    };

    typedef bool (*FCreateContextCallback)(void* window);

    typedef void (*FDestroyContextCallback)();

    typedef void (*FBeginFrameCallback)();

    typedef void (*FEndFrameCallback)();

    struct RenderBackend
    {
        FCreateContextCallback  FInitialize;
        FDestroyContextCallback FDestroy;
        FBeginFrameCallback     FBeginFrame;
        FEndFrameCallback       FEndFrame;

        RENDER_BACKEND Backend = RENDER_BACKEND_NONE;
    };

    void GetGLBindings(RenderBackend* backend);
}
