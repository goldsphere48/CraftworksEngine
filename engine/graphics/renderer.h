#pragma once

#include "renderer_backend.h"

namespace cw::graphics
{
    struct ContextParams
    {
        void*          Window;
        RENDER_BACKEND Backend;
    };

    struct Context;
    
    Context* Create(const ContextParams* params);

    void Destroy(Context* ctx);

    void BeginFrame(Context* ctx);

    void EndFrame(Context* ctx);
}
