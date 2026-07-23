#pragma once

#include "renderer_backend.h"

#include <stdint.h>

namespace cw::graphics
{
    struct GraphicsParams
    {
        void*          Window;
        RENDER_BACKEND_TYPE Backend;
    };

    struct GraphicsContext;
    
    GraphicsContext* Create(const GraphicsParams* params);

    void Destroy(GraphicsContext* ctx);

    void BeginFrame();

    void EndFrame();

    HPipeline CreatePipeline(const char* path);

    void DestroyPipeline(HPipeline pipeline);

    void DrawMesh(const GraphicsContext* ctx, const Mesh* mesh);

    Mesh* CreateMesh(const void* vertices, size_t vertices_size, const uint32_t* indices, size_t index_count);

    void DestroyMesh(const Mesh* mesh);
}
