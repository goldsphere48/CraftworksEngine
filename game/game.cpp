#include "engine/engine.h"

using namespace cw;
using namespace cw::engine;

struct GameState
{
    graphics::HPipeline Pipeline;
    graphics::Mesh* Mesh;
} static g_State;

void CW_AppInitialize(const Engine* engine)
{
    static const float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 1.0f,
    };

    static const uint32 indices[] = {0, 1, 2};

    g_State.Mesh = graphics::CreateMesh(vertices, sizeof(vertices), indices, 3);
    g_State.Pipeline = graphics::CreatePipeline("shaders/solid_color.ppl");
}

void CW_AppUpdate(const Engine* engine)
{
    graphics::DrawMesh(g_State.Pipeline, g_State.Mesh);
}

void CW_AppDestroy(const Engine* engine)
{
    graphics::DestroyPipeline(g_State.Pipeline);
}
