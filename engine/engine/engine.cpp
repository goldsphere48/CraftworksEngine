#include "engine.h"

#include "logger/log.h"
#include "filesystem/filesystem.h"

namespace cw::engine
{
    static void DestroyEngine(const Engine* engine);
        
    static void OnWindowClose(void* userData)
    {
        Engine* engine = static_cast<Engine*>(userData);
        engine->IsRunning = false;
    }
    
    static Engine* CreateEngine()
    {
        Engine* engine = new Engine();
        
        cw::platform::PlatformParams pp;
        pp.WindowTitle = "CW Engine";
        pp.WindowWidth = 1024;
        pp.WindowHeight = 768;
        pp.WindowCloseCallback = OnWindowClose;
        pp.WindowCloseCallbackUserData = engine;
        engine->Platform = cw::platform::Create(&pp);

        if (!engine->Platform)
        {
            DestroyEngine(engine);
            return nullptr;
        }

        cw::fs::Initialize();
        
        cw::graphics::GraphicsParams gp;
        gp.Window = cw::platform::GetNativeWindowHandle(engine->Platform);
        gp.Backend = cw::graphics::RENDER_BACKEND_OPENGL;
        engine->Graphics = cw::graphics::Create(&gp);
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
            cw::graphics::Destroy(engine->Graphics);
        }

        if (engine->Platform)
        {
            cw::platform::Destroy(engine->Platform);
        }

        cw::fs::Shutdown();
        
        delete engine;
    }

    static void UpdateEngine(const Engine* engine)
    {
        cw::platform::PollEvents();

        cw::graphics::BeginFrame();
        cw::graphics::EndFrame();
    }
    
    static int RunLoop(int argc, char** argv)
    {
        cw::log::Initialize();

        Engine* engine = CreateEngine();
        if (!engine)
        {
            return 0;
        }
        
        while (engine->IsRunning)
        {
            UpdateEngine(engine);
        }
        
        DestroyEngine(engine);

        return 0;
    }
}

int EngineMain(int argc, char** argv)
{
    return cw::engine::RunLoop(argc, argv);
}
