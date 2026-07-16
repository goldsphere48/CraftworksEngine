#include "renderer.h"

#include "logger/log.h"
#include "opengl/opengl_renderer.h"

namespace cw::graphics
{
    static RenderBackend g_Backend;
    
    struct GraphicsContext
    {
    
    };
    
    GraphicsContext* Create(const GraphicsParams* params)
    {
        g_Backend.BackendType = params->Backend;

        switch(g_Backend.BackendType)
        {
            case RENDER_BACKEND_OPENGL:
                GetGLBindings(&g_Backend);
                break;
            default:
                return nullptr;
        }

        if (!g_Backend.Initialize(params->Window))
        {
            g_Backend.Destroy();
            return nullptr;
        }

        GraphicsContext* ctx = new GraphicsContext;
        return ctx;
    }

    void BeginFrame()
    {
        g_Backend.BeginFrame();
    }

    void EndFrame()
    {
        g_Backend.EndFrame();
    }

    void Destroy(GraphicsContext* ctx)
    {
        g_Backend.Destroy();
        delete ctx;
    }
}
