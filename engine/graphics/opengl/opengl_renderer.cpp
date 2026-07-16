#include "opengl_renderer.h"

#include "debug/debug.h"
#include "graphics/renderer_backend.h"

#ifdef CW_PLATFORM_WINDOWS
    #include <windows.h>
#endif

#include <gl/gl.h>

namespace cw::graphics
{
    static GLPlatformContext* g_Context;

    static bool Initialize(void* window)
    {
        CW_ASSERT(!g_Context);

        g_Context = GLInitializePlatform(window);
        return g_Context != nullptr;
    }

    static void Destroy()
    {
        GLDestroyPlatform(g_Context);
        g_Context = nullptr;
    }

    static void BeginFrame()
    {
        glClearColor(0.3f, 0.3f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    static void EndFrame()
    {
        GLSwapBuffers(g_Context);
    }


    void GetGLBindings(RenderBackend* backend)
    {
        backend->FInitialize = Initialize;
        backend->FDestroy    = Destroy;
        backend->FBeginFrame = BeginFrame;
        backend->FEndFrame   = EndFrame;
    }
}
