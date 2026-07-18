#pragma once

namespace cw::fs
{
    struct FileBuffer;
    struct Provider;

    typedef bool(*FProviderRead)(void* self, const char* path, FileBuffer* out);
    typedef bool(*FProviderExist)(void* self, const char* path);
    typedef void(*FProviderDestroy)(Provider* self);

    struct Provider
    {
        FProviderRead    Read;
        FProviderExist   Exists;
        FProviderDestroy Destroy;
        void*            Self;
    };
}
