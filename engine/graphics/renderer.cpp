#include "renderer.h"

namespace cw::graphics
{
    struct Context
    {
        RenderBackend Renderer;
    };
    
    Context* Create(const ContextParams* params)
    {
        Context* ctx          = new Context;
        ctx->Renderer.Backend = params->Backend;

        switch(ctx->Renderer.Backend)
        {
            case RENDER_BACKEND_OPENGL:
                GetGLBindings(&ctx->Renderer);
                break;
            default:
                delete ctx;
                return nullptr;
        }

        if (!ctx->Renderer.FInitialize(params->Window))
        {
            delete ctx;
            return nullptr;
        }

        return ctx;
    }

    void BeginFrame(Context* ctx)
    {
        ctx->Renderer.FBeginFrame();
    }

    void EndFrame(Context* ctx)
    {
        ctx->Renderer.FEndFrame();
    }

    void Destroy(Context* ctx)
    {
        ctx->Renderer.FDestroy();
        delete ctx;
    }
}
