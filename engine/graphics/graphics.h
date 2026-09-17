#pragma once

#include "core/types.h"
#include "graphics_adapter.h"
#include "math/matrix.h"

// The device layer. It owns the adapter and the swapchain-level state, and
// nothing else: meshes, materials and pipelines-with-uniform-reflection all
// live one layer up, in cw::render.

namespace cw::graphics
{
    struct GraphicsParams
    {
        void*          Window;
        ADAPTER_FAMILY Family;
        Vec2i          Viewport;
    };

    struct GraphicsContext
    {
        GraphicsAdapter Adapter;
        Viewport        Viewport;
    };

    GraphicsContext* Create(const GraphicsParams* params);

    void Destroy(GraphicsContext* ctx);

    void BeginFrame(GraphicsContext* ctx);

    void EndFrame(GraphicsContext* ctx);

    void OnResize(GraphicsContext* ctx, int width, int height);

    HPipeline CreatePipeline(GraphicsContext* ctx, const PipelineDesc* desc);

    void DestroyPipeline(GraphicsContext* ctx, HPipeline pipeline);

    void BindPipeline(GraphicsContext* ctx, HPipeline pipeline);

    HUniformLocation GetUniformLocation(GraphicsContext* ctx, HPipeline pipeline, uint64 nameHash);

    void SetConstantV4(
        GraphicsContext* ctx,
        HPipeline        pipeline,
        const float*     data,
        usize            count,
        HUniformLocation location
    );

    void SetConstantM4(
        GraphicsContext* ctx,
        HPipeline        pipeline,
        const float*     data,
        usize            count,
        HUniformLocation location
    );

    HBuffer CreateBuffer(GraphicsContext* ctx, const BufferDesc* desc);

    void DestroyBuffer(GraphicsContext* ctx, HBuffer buffer);

    void Draw(GraphicsContext* ctx, const DrawCall* draw);
}
