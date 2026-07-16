#pragma once

namespace cw::graphics
{
    struct RenderBackend;

    bool GLInitializePlatform(void* window);
    
    void GLDestroyPlatform();

    void GLSwapBuffers();

    void GetGLBindings(RenderBackend* backend);
}
