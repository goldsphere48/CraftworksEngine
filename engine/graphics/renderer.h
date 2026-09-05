#pragma once

#include "core/types.h"
#include "math/matrix.h"
#include "renderer_backend.h"

namespace cw::graphics
{
    struct PipelineManager;
    struct MaterialContext;

    // Composed by this layer out of backend buffers. IndexCount is kept here
    // rather than inside the backend buffer so the draw can be described
    // without asking the backend to hand back resource metadata.
    struct Mesh
    {
        HBuffer Vertices;
        HBuffer Indices;
        uint32  IndexCount = 0;
    };

    struct GraphicsParams
    {
        void*               Window;
        RENDER_BACKEND_TYPE Backend;
        Vec2i               Viewport;
    };

    struct GraphicsContext
    {
        RenderBackend    Backend;
        Viewport         Viewport;
        PipelineManager* PipelineManager;
        MaterialContext* MaterialContext;
    };

    GraphicsContext* Create(const GraphicsParams* params);

    void Destroy(GraphicsContext* ctx);

    void BeginFrame(GraphicsContext* ctx);

    void EndFrame(GraphicsContext* ctx);

    void OnResize(GraphicsContext* ctx, int width, int height);

    HPipeline CreatePipeline(GraphicsContext* ctx, const PipelineDesc* desc);

    void DestroyPipeline(GraphicsContext* ctx, HPipeline pipeline);

    void BindPipeline(GraphicsContext* ctx, HPipeline pipeline);

    void GetUniform(GraphicsContext* ctx, HPipeline pipeline, uint64 nameHash, HUniform* outUniform);

    void SetUniformFloat(GraphicsContext* ctx, HUniform uniform, float value);

    void SetUniformVec2(GraphicsContext* ctx, HUniform uniform, Vec2 value);

    void SetUniformVec3(GraphicsContext* ctx, HUniform uniform, Vec3 value);

    void SetUniformVec4(GraphicsContext* ctx, HUniform uniform, Vec4 value);

    void SetUniformMat4(GraphicsContext* ctx, HUniform uniform, const Mat4* value);

    HUniformBuffer CreateUniformBuffer(GraphicsContext* ctx);

    void DestroyUniformBuffer(GraphicsContext* ctx, HUniformBuffer buffer);

    void BindUniformBuffer(GraphicsContext* ctx, HUniformBuffer buffer);

    Mesh* CreateMesh(
        GraphicsContext* ctx,
        const void*      vertices,
        usize            verticesSize,
        const uint32*    indices,
        uint32           indexCount
    );

    void DestroyMesh(GraphicsContext* ctx, const Mesh* mesh);

    void DrawMesh(GraphicsContext* ctx, const Mesh* mesh, HPipeline pipeline);
}
