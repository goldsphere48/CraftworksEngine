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

        ctx->Pipeline = CreatePipeline("shaders/solid_color.ppl");

        return ctx;
    }

    HPipeline CreatePipeline(const char* path)
    {
        assets::PipelineAsset* pipelineAsset = assets::LoadPipelineAsset(path);
        if (!pipelineAsset)
        {
            return nullptr;
        }

        static const VertexAttribute attributes[] = {
            {0, VERTEX_FORMAT_FLOAT3},
            {1, VERTEX_FORMAT_FLOAT3},
            {2, VERTEX_FORMAT_FLOAT4},
            {3, VERTEX_FORMAT_FLOAT2},
        };

        PipelineDesc desc = {
            .Source = {
                .VertexSource   = (const char*)pipelineAsset->VertexBuffer->Data,
                .FragmentSource = (const char*)pipelineAsset->FragmentBuffer->Data,
            },
            .Layout = {
                .Attributes     = attributes,
                .AttributeCount = 4,
                .Stride         = 48,
            },
        };

        HPipeline pipeline = g_Backend.CreatePipeline(&desc);

        assets::FreePipelineAsset(pipelineAsset);

        return pipeline;
    }

    void DestroyPipeline(HPipeline pipeline)
    {
        g_Backend.DestroyPipeline(pipeline);
    }


    void BeginFrame()
    {
        g_Backend.BeginFrame();
    }

    void EndFrame()
    {
        g_Backend.EndFrame();
    }

    void DrawMesh(const GraphicsContext* ctx, const Mesh* mesh)
    {
        g_Backend.BindPipeline(ctx->Pipeline);
        g_Backend.DrawMesh(mesh, ctx->Pipeline);
    }

    Mesh* CreateMesh(const void* vertices, size_t vertices_size, const uint32_t* indices, size_t index_count)
    {
        BufferDesc vertexDesc = {
            .Count = 0,
            .Size  = vertices_size,
            .Data  = (void*)vertices,
        };

        BufferDesc indexDesc = {
            .Count = index_count,
            .Size  = index_count * sizeof(uint32_t),
            .Data  = (void*)indices,
        };

        Mesh* mesh     = new Mesh;
        mesh->Vertices = g_Backend.CreateBuffer(&vertexDesc);
        mesh->Indicies = g_Backend.CreateBuffer(&indexDesc);
        return mesh;
    }

    void DestroyMesh(const Mesh* mesh)
    {
        g_Backend.DeleteBuffer(mesh->Vertices);
        g_Backend.DeleteBuffer(mesh->Indicies);
        delete mesh;
    }

    void Destroy(GraphicsContext* ctx)
    {
        if (ctx->Pipeline)
        {
            DestroyPipeline(ctx->Pipeline);
        }
        
        g_Backend.Destroy();

        delete ctx;
    }
}
