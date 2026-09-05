#include "renderer.h"

#include "material.h"
#include "opengl/opengl_renderer.h"
#include "pipeline_manager.h"

namespace cw::graphics
{
    static bool BindBackend(RenderBackend* backend, RENDER_BACKEND_TYPE type)
    {
        backend->BackendType = type;

        switch (type)
        {
            case RENDER_BACKEND_OPENGL:
                GetGLBindings(backend);
                return true;
            default:
                return false;
        }
    }

    GraphicsContext* Create(const GraphicsParams* params)
    {
        GraphicsContext* ctx = new GraphicsContext{};

        if (!BindBackend(&ctx->Backend, params->Backend))
        {
            delete ctx;
            return nullptr;
        }

        if (!ctx->Backend.Initialize(params->Window))
        {
            delete ctx;
            return nullptr;
        }

        ctx->PipelineManager = CreatePipelineManager(ctx);
        ctx->MaterialContext = CreateContext(ctx->PipelineManager);

        OnResize(ctx, params->Viewport.X, params->Viewport.Y);

        return ctx;
    }

    void Destroy(GraphicsContext* ctx)
    {
        DestroyContext(ctx->MaterialContext);
        DestroyPipelineManager(ctx->PipelineManager);
        ctx->Backend.Destroy();

        delete ctx;
    }

    void OnResize(GraphicsContext* ctx, int width, int height)
    {
        ctx->Viewport.Size.X = width;
        ctx->Viewport.Size.Y = height;

        ctx->Backend.UpdateViewport(width, height);
    }

    void BeginFrame(GraphicsContext* ctx)
    {
        ctx->Backend.BeginFrame();
    }

    void EndFrame(GraphicsContext* ctx)
    {
        ctx->Backend.EndFrame();
    }

    HPipeline CreatePipeline(GraphicsContext* ctx, const PipelineDesc* desc)
    {
        return ctx->Backend.CreatePipeline(desc);
    }

    void DestroyPipeline(GraphicsContext* ctx, HPipeline pipeline)
    {
        ctx->Backend.DestroyPipeline(pipeline);
    }

    void BindPipeline(GraphicsContext* ctx, HPipeline pipeline)
    {
        ctx->Backend.BindPipeline(pipeline);
    }

    void GetUniform(GraphicsContext* ctx, HPipeline pipeline, uint64 nameHash, HUniform* outUniform)
    {
        ctx->Backend.GetUniform(pipeline, nameHash, outUniform);
    }

    void SetUniformFloat(GraphicsContext* ctx, HUniform uniform, float value)
    {
        ctx->Backend.SetFloat(uniform, value);
    }

    void SetUniformVec2(GraphicsContext* ctx, HUniform uniform, Vec2 value)
    {
        ctx->Backend.SetVec2(uniform, value);
    }

    void SetUniformVec3(GraphicsContext* ctx, HUniform uniform, Vec3 value)
    {
        ctx->Backend.SetVec3(uniform, value);
    }

    void SetUniformVec4(GraphicsContext* ctx, HUniform uniform, Vec4 value)
    {
        ctx->Backend.SetVec4(uniform, value);
    }

    void SetUniformMat4(GraphicsContext* ctx, HUniform uniform, const Mat4* value)
    {
        ctx->Backend.SetMat4(uniform, value->Data);
    }

    Mesh* CreateMesh(
        GraphicsContext* ctx,
        const void*      vertices,
        usize            verticesSize,
        const uint32*    indices,
        uint32           indexCount
    )
    {
        const BufferDesc vertexDesc = {
            .Size = verticesSize,
            .Data = vertices,
        };

        const BufferDesc indexDesc = {
            .Size = indexCount * sizeof(uint32),
            .Data = indices,
        };

        Mesh* mesh       = new Mesh;
        mesh->Vertices   = ctx->Backend.CreateBuffer(&vertexDesc);
        mesh->Indices    = ctx->Backend.CreateBuffer(&indexDesc);
        mesh->IndexCount = indexCount;
        return mesh;
    }

    void DestroyMesh(GraphicsContext* ctx, const Mesh* mesh)
    {
        ctx->Backend.DeleteBuffer(mesh->Vertices);
        ctx->Backend.DeleteBuffer(mesh->Indices);
        delete mesh;
    }

    void DrawMesh(GraphicsContext* ctx, const Mesh* mesh, HPipeline pipeline)
    {
        const DrawCall draw = {
            .Pipeline   = pipeline,
            .Vertices   = mesh->Vertices,
            .Indices    = mesh->Indices,
            .IndexCount = mesh->IndexCount,
        };

        ctx->Backend.Draw(&draw);
    }
}
