#include "engine.h"

#include "input/input.h"
#include "filesystem/filesystem.h"
#include "logger/log.h"

namespace cw::engine
{
    static void OnPlatformEvent(const platform::Event* event, void* userData)
    {
        Engine* engine    = static_cast<Engine*>(userData);

        if (event->Type == platform::EVENT_WINDOW_CLOSE)
        {
            engine->IsRunning = false;
            return;
        }

        if (event->Type == platform::EVENT_WINDOW_RESIZE)
        {
            graphics::OnResize(engine->Graphics, event->Window.ViewportWidth, event->Window.ViewportHeight);
            return;
        }

        input::HandleEvent(event);
    }

    static Engine* CreateEngine()
    {
        Engine* engine = new Engine();

        platform::PlatformParams pp;
        pp.WindowTitle           = "CW Engine";
        pp.EventCallback         = OnPlatformEvent;
        pp.EventCallbackUserData = engine;
        engine->Platform         = platform::Create(&pp);

        if (!engine->Platform)
        {
            CW_ERROR("Failed to create platform");
            delete engine;
            return nullptr;
        }

        if (!fs::Initialize())
        {
            CW_ERROR("Failed to initialize filesystem");
            platform::Destroy(engine->Platform);
            delete engine;
            return nullptr;
        }

        graphics::GraphicsParams gp;
        gp.Window        = platform::GetNativeWindowHandle(engine->Platform);
        gp.Backend       = graphics::RENDER_BACKEND_OPENGL;
        gp.Viewport      = platform::GetViewportSize(engine->Platform);
        engine->Graphics = graphics::Create(&gp);

        if (!engine->Graphics)
        {
            CW_ERROR("Failed to create graphics");
            fs::Shutdown();
            platform::Destroy(engine->Platform);
            delete engine;
            return nullptr;
        }

        return engine;
    }

    static void DestroyEngine(const Engine* engine)
    {
        graphics::Destroy(engine->Graphics);

        fs::Shutdown();

        platform::Destroy(engine->Platform);

        delete engine;
    }

    static void UpdateEngine(const Engine* engine)
    {
        platform::PollEvents();
        graphics::BeginFrame();
        CW_AppUpdate(engine);
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
            return 1;
        }

        CW_AppInitialize(engine);

        while (engine->IsRunning)
        {
            UpdateEngine(engine);
        }

        CW_AppDestroy(engine);
        
        DestroyEngine(engine);

        return 0;
    }
}

int EngineMain(int argc, char** argv)
{
    return cw::engine::RunLoop(argc, argv);
}
