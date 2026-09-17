#include "mesh.h"

#include "render_private.h"

namespace cw::render
{
    Mesh* CreateMesh(
        RenderContext* ctx,
        const void*    vertices,
        usize          verticesSize,
        const uint32*  indices,
        uint32         indexCount
    )
    {
        const graphics::BufferDesc vertexDesc = {
            .Size = verticesSize,
            .Data = vertices,
        };

        const graphics::BufferDesc indexDesc = {
            .Size = indexCount * sizeof(uint32),
            .Data = indices,
        };

        Mesh* mesh       = new Mesh;
        mesh->Vertices   = graphics::CreateBuffer(ctx->Graphics, &vertexDesc);
        mesh->Indices    = graphics::CreateBuffer(ctx->Graphics, &indexDesc);
        mesh->IndexCount = indexCount;
        return mesh;
    }

    void DestroyMesh(RenderContext* ctx, const Mesh* mesh)
    {
        graphics::DestroyBuffer(ctx->Graphics, mesh->Vertices);
        graphics::DestroyBuffer(ctx->Graphics, mesh->Indices);
        delete mesh;
    }
}
