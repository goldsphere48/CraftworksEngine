#pragma once

#include "renderer_backend.h"
#include "core/types.h"

namespace cw::graphics
{
    struct GraphicsParams
    {
        void*               Window;
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

    Mesh* CreateMesh(
        const void*   vertices,
        usize         vertices_size,
        const uint32* indices,
        usize         index_count
    );

    void DestroyMesh(const Mesh* mesh);
}
