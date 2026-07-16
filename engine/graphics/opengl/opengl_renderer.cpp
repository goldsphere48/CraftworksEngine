#include "opengl_renderer.h"

#include "debug/debug.h"
#include "graphics/renderer_backend.h"

#ifdef CW_PLATFORM_WINDOWS
    #include <windows.h>
#endif

#include <gl/gl.h>

namespace cw::graphics
{
    static bool Initialize(void* window)
    {
        bool result = GLInitializePlatform(window);
        return result;
    }

    static void Destroy()
    {
        GLDestroyPlatform();
    }

    static void BeginFrame()
    {
        glClearColor(0.3f, 0.3f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    static void EndFrame()
    {
        GLSwapBuffers();
    }


    void GetGLBindings(RenderBackend* backend)
    {
        backend->Initialize = Initialize;
        backend->Destroy    = Destroy;
        backend->BeginFrame = BeginFrame;
        backend->EndFrame   = EndFrame;
    }
}
