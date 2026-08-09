#include "engine.h"

#include "input/input.h"
#include "filesystem/filesystem.h"
#include "logger/log.h"

namespace cw::engine
{
    graphics::Mesh* g_Mesh = nullptr;

    static void DestroyEngine(const Engine* engine);

    static void OnPlatformEvent(const platform::Event* event, void* userData)
    {
        Engine* engine    = static_cast<Engine*>(userData);

        if (event->Type == platform::EVENT_WINDOW_CLOSE)
        {
            engine->IsRunning = false;
            return;
        }

        input::HandleEvent(event);
    }

    static Engine* CreateEngine()
    {
        Engine* engine = new Engine();

        platform::PlatformParams pp;
        pp.WindowTitle           = "CW Engine";
        pp.WindowWidth           = 1024;
        pp.WindowHeight          = 768;
        pp.EventCallback         = OnPlatformEvent;
        pp.EventCallbackUserData = engine;
        engine->Platform         = platform::Create(&pp);

        if (!engine->Platform)
        {
            DestroyEngine(engine);
            return nullptr;
        }

        fs::Initialize();

        graphics::GraphicsParams gp;
        gp.Window        = platform::GetNativeWindowHandle(engine->Platform);
        gp.Backend       = graphics::RENDER_BACKEND_OPENGL;
        engine->Graphics = graphics::Create(&gp);
        if (!engine->Graphics)
        {
            DestroyEngine(engine);
            return nullptr;
        }

        return engine;
    }

    static void DestroyEngine(const Engine* engine)
    {
        if (engine->Graphics)
        {
            graphics::Destroy(engine->Graphics);
        }

        if (engine->Platform)
        {
            platform::Destroy(engine->Platform);
        }

        fs::Shutdown();

        delete engine;
    }

    static void UpdateEngine(const Engine* engine)
    {
        platform::PollEvents();

        graphics::BeginFrame();
        graphics::DrawMesh(engine->Graphics, g_Mesh);
        graphics::EndFrame();
        input::EndFrame();
    }

    static int RunLoop(int argc, char** argv)
    {
        log::Initialize();
        input::Initialize();

        Engine* engine = CreateEngine();
        if (!engine)
        {
            return 0;
        }

        static const float vertices[] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
            0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 1.0f,
        };

        static const uint32 indices[] = {0, 1, 2};

        g_Mesh = graphics::CreateMesh(vertices, sizeof(vertices), indices, 3);

        while (engine->IsRunning)
        {
            UpdateEngine(engine);
        }

        graphics::DestroyMesh(g_Mesh);

        DestroyEngine(engine);

        return 0;
    }
}

int EngineMain(int argc, char** argv)
{
    return cw::engine::RunLoop(argc, argv);
}
