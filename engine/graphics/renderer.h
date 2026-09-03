#pragma once

#include "core/types.h"
#include "material.h"
#include "renderer_backend.h"

namespace cw::graphics
{
    struct PipelineManager;

    struct GraphicsParams
    {
        void*               Window;
        RENDER_BACKEND_TYPE Backend;
        Vec2i               Viewport;
    };

    struct GraphicsContext
    {
        Viewport                   Viewport;
        PipelineManager*           PipelineManager;
        MaterialContext*           MaterialContext;
    };

    GraphicsContext* Create(const GraphicsParams* params);

    void Destroy(GraphicsContext* ctx);

    void BeginFrame();

    void EndFrame();

    void OnResize(GraphicsContext* ctx, int width, int height);

    HPipeline CreatePipeline(const PipelineDesc* desc);

    void DestroyPipeline(HPipeline pipeline);

    void GetUniform(const HPipeline pipeline, uint64 nameHash, HUniform* outUniform);

    void DrawMesh(const Material* material, const Mesh* mesh);

    Mesh* CreateMesh(
        const void*   vertices,
        usize         verticesSize,
        const uint32* indices,
        usize         indexCount
    );

    void DestroyMesh(const Mesh* mesh);
}
