#pragma once

#include "graphics/graphics.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "pipeline.h"
#include "render.h"

// Shared between the translation units of the render layer. Not installed for
// engine code above it: everything outside cw::render goes through render.h.

namespace cw::render
{
    // Packed (MajorOrder, BatchKey, Order) so ordering is one integer compare.
    struct RenderEntry
    {
        uint64 SortKey = 0;
        uint32 Index   = 0;
    };

    struct RenderContext
    {
        graphics::GraphicsContext* Graphics       = nullptr;
        RenderObject*              Objects        = nullptr;
        RenderEntry*               Entries        = nullptr;
        usize                      ObjectCount    = 0;
        usize                      ObjectCapacity = 0;
        Camera                     Camera         = {};
    };

    // The single place that turns a stored value into a backend write. Everything
    // narrower than a vec4 was promoted on the way in; the backend unpacks it again
    // from the type it recorded when the pipeline was built.
    inline void ApplyUniform(
        RenderContext*         ctx,
        const Pipeline*        pipeline,
        const PipelineUniform& uniform,
        const Vec4*            elements
    )
    {
        if (!graphics::IsValidLocation(uniform.Location))
        {
            return;
        }

        if (uniform.Type == graphics::UNIFORM_TYPE_MAT4)
        {
            graphics::SetConstantM4(
                ctx->Graphics,
                pipeline->BackendPipeline,
                elements[0].Data,
                1,
                uniform.Location
            );
            return;
        }

        graphics::SetConstantV4(
            ctx->Graphics,
            pipeline->BackendPipeline,
            elements[0].Data,
            1,
            uniform.Location
        );
    }

    inline void ApplyBuiltinMat4(
        RenderContext*   ctx,
        const Pipeline*  pipeline,
        BUILTIN_UNIFORMS builtin,
        const Mat4*      value
    )
    {
        const graphics::HUniformLocation location = pipeline->Builtins[builtin];
        if (!graphics::IsValidLocation(location))
        {
            return;
        }

        graphics::SetConstantM4(ctx->Graphics, pipeline->BackendPipeline, value->Data, 1, location);
    }
}
