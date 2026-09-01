#pragma once

#include "core/types.h"
#include "renderer_backend.h"
#include "material.h"

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
        Viewport         Viewport;
        PipelineManager* PipelineManager;
        material::MaterialContext* MaterialContext;
    };

    GraphicsContext* Create(const GraphicsParams* params);

    void Destroy(GraphicsContext* ctx);

    void BeginFrame();

    void EndFrame();

    void OnResize(GraphicsContext* ctx, int width, int height);

    HPipeline CreatePipeline(const PipelineDesc* desc);

    void DestroyPipeline(HPipeline pipeline);

    void GetUniform(const HPipeline pipeline, uint64 name_hash, HUniform* out_uniform);

    void SetFloat(const HUniform uniform, float value);

    void SetVec2(const HUniform uniform, Vec2 value);

    void SetVec3(const HUniform uniform, Vec3 value);

    void SetVec4(const HUniform uniform, Vec4 value);

    void SetMat4(const HUniform uniform, const float* value);

    void DrawMesh(const material::Material* material, const Mesh* mesh);

    Mesh* CreateMesh(
        const void*   vertices,
        usize         vertices_size,
        const uint32* indices,
        usize         index_count
    );

    void DestroyMesh(const Mesh* mesh);
}
