#pragma once

#include "core/types.h"
#include "graphics/graphics.h"
#include "math/matrix.h"
#include "mesh.h"

// The render layer. It owns everything the device layer refuses to know about:
// pipelines with uniform reflection, materials, meshes and the per-frame list of
// things to draw. It talks to cw::graphics and never to the filesystem.

namespace cw::render
{
    struct RenderContext;
    struct Material;
    struct ConstantBuffer;

    struct Camera
    {
        Mat4 View       = math::mat4::Identity;
        Mat4 Projection = math::mat4::Identity;
    };

    enum MAJOR_ORDER
    {
        MAJOR_ORDER_BEFORE = 0,
        MAJOR_ORDER_WORLD  = 1,
        MAJOR_ORDER_AFTER  = 2,
    };

    struct RenderObject
    {
        const Material* Material       = nullptr;
        const Mesh*     Mesh           = nullptr;
        ConstantBuffer* Constants      = nullptr;   // optional per-instance overrides
        Mat4            WorldTransform = math::mat4::Identity;
        uint32          Order          = 0;
        uint8           MajorOrder     = MAJOR_ORDER_WORLD;
    };

    struct RenderParams
    {
        graphics::GraphicsContext* Graphics         = nullptr;
        usize                      MaxRenderObjects = 1024;
    };

    RenderContext* CreateRenderContext(const RenderParams* params);

    void DestroyRenderContext(RenderContext* ctx);

    graphics::GraphicsContext* GetGraphicsContext(RenderContext* ctx);

    void SetCamera(RenderContext* ctx, const Camera* camera);

    void BeginFrame(RenderContext* ctx);

    // Copies the object into the frame's list. The caller keeps ownership of the
    // material, mesh and constant buffer it points at, and must keep them alive
    // until the frame is drawn.
    void AddRenderObject(RenderContext* ctx, const RenderObject* object);

    // Sorts the frame's objects and submits them, skipping redundant binds.
    void DrawRenderList(RenderContext* ctx);

    void EndFrame(RenderContext* ctx);
}
