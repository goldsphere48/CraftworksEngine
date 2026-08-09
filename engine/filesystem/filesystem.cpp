#include "filesystem.h"

#include "logger/log.h"
#include "debug/debug.h"
#include "platform/platform.h"
#include "file_provider.h"

#include <stdio.h>

namespace cw::fs
{
    constexpr int MAX_PROVIDERS = 512;

    struct FileSystem
    {
        usize     ProvidersCount;
        Provider* Providers[MAX_PROVIDERS];
        char      AssetsRoot[CW_MAX_PATH];
        bool      IsInitialized = false;
    };

    FileSystem g_fs;

    static bool ResolveAssetsRoot(char* out, usize size)
    {
#ifdef CW_ASSETS_DIR
        snprintf(out, size, "%s", CW_ASSETS_DIR);
        return true;
#else
        char exeDir[CW_MAX_PATH];
        if (!platform::GetExeDir(exeDir, sizeof(exeDir)))
        {
            CW_ERROR("Failed to resolve assets root");
            return false;
        }
        snprintf(out, size, "%s", exeDir);
        return true;
#endif
    }

    bool Initialize()
    {
        CW_ASSERT(!g_fs.IsInitialized);

        if (!ResolveAssetsRoot(g_fs.AssetsRoot, sizeof(g_fs.AssetsRoot)))
        {
            return false;
        }

        g_fs.IsInitialized  = true;
        g_fs.ProvidersCount = 0;

        CW_INFO("Assets root: %s", g_fs.AssetsRoot);

        Mount(CreateFileProvider(g_fs.AssetsRoot));

        return true;
    }

    void Shutdown()
    {
        CW_ASSERT(g_fs.IsInitialized);
        
        g_fs.IsInitialized = false;

        for (usize i = 0; i < g_fs.ProvidersCount; ++i)
        {
            g_fs.Providers[i]->Destroy(g_fs.Providers[i]);
        }

        g_fs.ProvidersCount = 0;
    }

    const char* GetAssetsPath()
    {
        CW_ASSERT(g_fs.IsInitialized);
        
        return g_fs.AssetsRoot;
    }

    void Mount(Provider* provider)
    {
        CW_ASSERT(g_fs.IsInitialized);
        CW_ASSERT(provider != nullptr);

        if (g_fs.ProvidersCount >= MAX_PROVIDERS)
        {
            return;
        }

        g_fs.Providers[g_fs.ProvidersCount++] = provider;
    }

    FileBuffer* ReadFile(const char* virtual_path)
    {
        CW_ASSERT(g_fs.IsInitialized);
        
        for (usize i = 0; i < g_fs.ProvidersCount; ++i)
        {
            Provider* p = g_fs.Providers[i];
            if (p->Exists(p->Self, virtual_path))
            {
                FileBuffer* file = new FileBuffer;
                if (p->Read(p->Self, virtual_path, file))
                {
                    return file;
                }
                delete file;
            }
        }

        CW_ERROR("Failed to get resource %s", virtual_path);
        return nullptr;
    }

    void FreeFile(FileBuffer* file)
    {
        CW_ASSERT(g_fs.IsInitialized);
        
        //NOTE: Allocation was in platform layer, but free in filestem.
        free(file->Data);
        file->Data = nullptr;
        file->Size = 0;

        delete file;
    }
}
