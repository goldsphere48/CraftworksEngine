#pragma once

namespace cw::graphics
{
    struct GraphicsAdapter;

    bool GLInitializePlatform(void* window);
    
    void GLDestroyPlatform();

    void GLSwapBuffers();

    void GetGLAdapter(GraphicsAdapter* adapter);
}
