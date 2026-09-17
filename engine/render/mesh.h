#pragma once

#include "core/types.h"
#include "graphics/graphics_types.h"

namespace cw::render
{
    struct RenderContext;

    // Composed by this layer out of backend buffers. IndexCount is kept here
    // rather than inside the backend buffer so the draw can be described
    // without asking the backend to hand back resource metadata.
    struct Mesh
    {
        graphics::HBuffer Vertices;
        graphics::HBuffer Indices;
        uint32            IndexCount = 0;
    };

    Mesh* CreateMesh(
        RenderContext* ctx,
        const void*    vertices,
        usize          verticesSize,
        const uint32*  indices,
        uint32         indexCount
    );

    void DestroyMesh(RenderContext* ctx, const Mesh* mesh);
}
