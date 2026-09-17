#include "graphics.h"

#include "null/null_graphics.h"
#include "opengl/opengl_graphics.h"

namespace cw::graphics
{
    static bool BindAdapter(GraphicsAdapter* adapter, ADAPTER_FAMILY family)
    {
        adapter->Family = family;

        switch (family)
        {
            case ADAPTER_FAMILY_OPENGL:
                GetGLAdapter(adapter);
                return true;
            case ADAPTER_FAMILY_NULL:
                GetNullAdapter(adapter);
                return true;
            default:
                return false;
        }
    }

    GraphicsContext* Create(const GraphicsParams* params)
    {
        GraphicsContext* ctx = new GraphicsContext{};

        if (!BindAdapter(&ctx->Adapter, params->Family))
        {
            delete ctx;
            return nullptr;
        }

        if (!ctx->Adapter.Initialize(params->Window))
        {
            delete ctx;
            return nullptr;
        }

        OnResize(ctx, params->Viewport.X, params->Viewport.Y);

        return ctx;
    }

    void Destroy(GraphicsContext* ctx)
    {
        ctx->Adapter.Destroy();

        delete ctx;
    }

    void OnResize(GraphicsContext* ctx, int width, int height)
    {
        ctx->Viewport.Size.X = width;
        ctx->Viewport.Size.Y = height;

        ctx->Adapter.UpdateViewport(width, height);
    }

    void BeginFrame(GraphicsContext* ctx)
    {
        ctx->Adapter.BeginFrame();
    }

    void EndFrame(GraphicsContext* ctx)
    {
        ctx->Adapter.EndFrame();
    }

    HPipeline CreatePipeline(GraphicsContext* ctx, const PipelineDesc* desc)
    {
        return ctx->Adapter.CreatePipeline(desc);
    }

    void DestroyPipeline(GraphicsContext* ctx, HPipeline pipeline)
    {
        ctx->Adapter.DestroyPipeline(pipeline);
    }

    void BindPipeline(GraphicsContext* ctx, HPipeline pipeline)
    {
        ctx->Adapter.BindPipeline(pipeline);
    }

    HUniformLocation GetUniformLocation(GraphicsContext* ctx, HPipeline pipeline, uint64 nameHash)
    {
        return ctx->Adapter.GetUniformLocation(pipeline, nameHash);
    }

    void SetConstantV4(
        GraphicsContext* ctx,
        HPipeline        pipeline,
        const float*     data,
        usize            count,
        HUniformLocation location
    )
    {
        ctx->Adapter.SetConstantV4(pipeline, data, count, location);
    }

    void SetConstantM4(
        GraphicsContext* ctx,
        HPipeline        pipeline,
        const float*     data,
        usize            count,
        HUniformLocation location
    )
    {
        ctx->Adapter.SetConstantM4(pipeline, data, count, location);
    }

    HBuffer CreateBuffer(GraphicsContext* ctx, const BufferDesc* desc)
    {
        return ctx->Adapter.CreateBuffer(desc);
    }

    void DestroyBuffer(GraphicsContext* ctx, HBuffer buffer)
    {
        ctx->Adapter.DeleteBuffer(buffer);
    }

    void Draw(GraphicsContext* ctx, const DrawCall* draw)
    {
        ctx->Adapter.Draw(draw);
    }
}
