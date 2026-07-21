#pragma once

namespace cw::fs
{
    struct Provider;

    struct FileBuffer
    {
        void* Data;
        size_t Size;
    };
    
    void Initialize();

    void Shutdown();

    const char* GetAssetsPath();

    void Mount(Provider* provider);

    FileBuffer* ReadFile(const char* virtual_path);

    void FreeFile(FileBuffer* file);
}
