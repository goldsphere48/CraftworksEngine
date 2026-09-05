#include "engine/engine.h"
#include "utils/hash.h"
#include "graphics/gfx.h"
#include "graphics/material.h"

#include <math.h>

using namespace cw;
using namespace cw::engine;
using namespace cw::math;
using namespace cw::graphics;

struct GameState
{
    Material* Material;
    Mesh* Mesh;
    float Time = 0.0f;
    Vec4 Color;
} static g_State;

void CW_AppInitialize(const Engine* engine)
{
    static const float vertices[] = {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    };

    static const uint32 indices[] = {0, 1, 2};

    g_State.Mesh = CreateMesh(engine->Graphics, vertices, sizeof(vertices), indices, 3);
    g_State.Material = LoadMaterial(engine->Graphics->MaterialContext, "materials/blue.mat");
}

void CW_AppUpdate(const Engine* engine)
{
    g_State.Time += 0.1f;
    float valuex = sin(g_State.Time);
    float valuey = cos(g_State.Time);
    float valuez = cos(sin(g_State.Time));
    g_State.Color = vec::Make<float>((float)valuex, (float)valuey, (float)valuez, 255.0f);
//    SetVec4(g_State.Material, utils::HashString("u_Color"), g_State.Color);
    gfx::DrawMesh(engine->Graphics, g_State.Material, g_State.Mesh);
}

void CW_AppDestroy(const Engine* engine)
{
    DestroyMesh(engine->Graphics, g_State.Mesh);
    DestroyMaterial(g_State.Material);
}
