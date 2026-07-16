#pragma once

#include "renderer_backend.h"

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
}
