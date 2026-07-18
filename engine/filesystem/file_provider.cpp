#include "file_provider.h"

#include "filesystem/provider.h"
#include "platform/platform.h"
#include "filesystem/filesystem.h"
#include "logger/log.h"

#include <stdio.h>

namespace cw::fs
{
    struct FileProvider
    {
        const char* Root;
    };

    static void JoinPath(FileProvider* provider, const char* path, char* out, size_t size)
    {
        const char* rel = path;
        while (*rel == '/' || *rel == '\\')
        {
            ++rel;
        }
        snprintf(out, size, "%s/%s", provider->Root, rel);
    }

    static bool FileProviderRead(void* self, const char* path, FileBuffer* out)
    {
        FileProvider* provider = (FileProvider*)self;

        char fullPath[CW_MAX_PATH];
        JoinPath(provider, path, fullPath, sizeof(fullPath));

        CW_INFO("Read from filesystem %s", path);

        if (!platform::ReadFileToBuffer(fullPath, &out->Data, &out->Size))
        {
            CW_ERROR("Failed to read file %s", fullPath);
            return false;
        }

        return true;
    }

    static bool FileProviderExist(void* self, const char* utf8_path)
    {
        FileProvider* provider = static_cast<FileProvider*>(self);

        char fullPath[CW_MAX_PATH];
        JoinPath(provider, utf8_path, fullPath, sizeof(fullPath));

        FILE* f = fopen(fullPath, "rb");
        if (f == nullptr)
        {
            return false;
        }
        fclose(f);
        return true;
    }

    static void FileProviderDestroy(Provider* provider)
    {
        delete static_cast<FileProvider*>(provider->Self);
        delete provider;
    }

    Provider* CreateFileProvider(const char* root_utf8)
    {
        FileProvider* fileProvider = new FileProvider;
        fileProvider->Root = root_utf8;

        Provider* provider = new Provider;
        provider->Read    = FileProviderRead;
        provider->Exists  = FileProviderExist;
        provider->Destroy = FileProviderDestroy;
        provider->Self    = fileProvider;

        return provider;
    }
}
