#pragma once

namespace cw::graphics
{
    struct GLPlatformContext;

    GLPlatformContext* GLInitializePlatform(void* window);
    
    void GLDestroyPlatform(GLPlatformContext* ctx);

    void GLSwapBuffers(GLPlatformContext* ctx);
}
