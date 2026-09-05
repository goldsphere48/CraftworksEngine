#pragma once

#include "material.h"
#include "renderer.h"

namespace cw::gfx
{
    struct Camera
    {
        Mat4 View;
        Mat4 Projection;
    };

    void DrawMesh(
        graphics::GraphicsContext* ctx,
        const graphics::Material*  material,
        const graphics::Mesh*      mesh
    );
}
