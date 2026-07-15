#include "engine.h"

#include "logger/log.h"

namespace cw::engine
{
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

        engine->Platform = cw::platform::CreateContext(&pp);
        if (!engine->Platform)
        {
            delete engine;
            return nullptr;
        }
        
        return engine;
    }

    static void DestroyEngine(const Engine* engine)
    {
        cw::platform::Destroy(engine->Platform);
        delete engine;
    }

    static void UpdateEngine(const Engine* engine)
    {
        cw::platform::PollEvents();
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
