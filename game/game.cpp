#include "assets/resources.h"
#include "engine/engine.h"
#include "render/constant_buffer.h"
#include "render/material.h"
#include "render/render.h"
#include "utils/hash.h"

#include <math.h>

using namespace cw;
using namespace cw::engine;
using namespace cw::math;
using namespace cw::render;

struct GameState
{
    Material*       Material;
    Mesh*           Mesh;
    ConstantBuffer* Constants;
    float           Time = 0.0f;
} static g_State;

void CW_AppInitialize(const Engine* engine)
{
    static const float vertices[] = {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.5f, -0.5f, 0.0f, 0.0f,
        1.0f,  0.0f,  1.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  1.0f,
    };

    static const uint32 indices[] = {0, 1, 2};

    g_State.Mesh      = CreateMesh(engine->Render, vertices, sizeof(vertices), indices, 3);
    g_State.Material  = assets::AcquireMaterial(engine->Assets, "materials/blue.mat");
    g_State.Constants = CreateConstantBuffer(4);

    const Camera camera = {
        .View       = mat4::Identity,
        .Projection = mat4::Identity,
    };

    SetCamera(engine->Render, &camera);
}

void CW_AppUpdate(const Engine* engine)
{
    g_State.Time += 0.1f;

    const Vec4 color = vec::Make<float>(
        (sinf(g_State.Time) + 1.0f) * 0.5f,
        (cosf(g_State.Time) + 1.0f) * 0.5f,
        cosf(sinf(g_State.Time)),
        1.0f
    );

    SetConstant(g_State.Constants, utils::HashString("u_Color"), &color, 1);

    RenderObject object = {};
    object.Material     = g_State.Material;
    object.Mesh         = g_State.Mesh;
    object.Constants    = g_State.Constants;

    AddRenderObject(engine->Render, &object);
}

void CW_AppDestroy(const Engine* engine)
{
    DestroyConstantBuffer(g_State.Constants);
    DestroyMesh(engine->Render, g_State.Mesh);
    assets::ReleaseMaterial(engine->Assets, g_State.Material);
}
