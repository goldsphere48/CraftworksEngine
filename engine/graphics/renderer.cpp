#include "renderer.h"

#include "assets/assets_manager.h"
#include "logger/log.h"
#include "opengl/opengl_renderer.h"

namespace cw::graphics
{
    static RenderBackend g_Backend;

    struct GraphicsContext
    {
        HPipeline Pipeline = nullptr;
    };

    GraphicsContext* Create(const GraphicsParams* params)
    {
        g_Backend.BackendType = params->Backend;

        switch (g_Backend.BackendType)
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

        assets::PipelineAsset* pipelineAsset = assets::LoadPipelineAsset("shaders/solid_color.ppl");

        if (pipelineAsset)
        {
            PipelineDesc desc = {
                .Source = {
                    .VertexSource   = (const char*)pipelineAsset->VertexBuffer->Data,
                    .FragmentSource = (const char*)pipelineAsset->FragmentBuffer->Data,
                },
            };
            
            ctx->Pipeline = g_Backend.CreatePipeline(&desc);

            cw::assets::FreePipelineAsset(pipelineAsset);
        }

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
        if (ctx->Pipeline)
        {
            g_Backend.DestroyPipeline(ctx->Pipeline);    
        }
        
        g_Backend.Destroy();

        delete ctx;
    }
}
