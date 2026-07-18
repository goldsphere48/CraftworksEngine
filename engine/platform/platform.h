#pragma once

constexpr unsigned int CW_MAX_PATH = 1024;

namespace cw::platform
{  
    struct PlatformContext;

    typedef void (*FWindowCloseCallback)(void* userData);

    struct PlatformParams
    {
        FWindowCloseCallback WindowCloseCallback;
        void*                WindowCloseCallbackUserData;

        int         WindowWidth;
        int         WindowHeight;
        const char* WindowTitle;
    };

    PlatformContext* Create(const PlatformParams* params);

    void Destroy(PlatformContext* ctx);

    void* GetNativeWindowHandle(PlatformContext* ctx);

    void PollEvents();

    bool GetExeDir(char* out_utf8, size_t size);

    bool ReadFileToBuffer(const char* utf8_path, void** out_data, size_t* out_size);
}
