#include "resource_cache.h"

#include "core/darray.h"
#include "logger/log.h"
#include "utils/hash.h"

#include <cstring>

namespace cw::assets
{
    struct ResourceEntry
    {
        void*  Resource;
        char*  Path;
        uint64 Hash;
        uint32 RefCount;
    };

    struct ResourceCache
    {
        void*                 UserContext;
        FDestroyResource      Destroy;
        darray<ResourceEntry> Entries;
    };

    static char* CopyString(const char* value)
    {
        const usize length = strlen(value);
        char*       copy   = new char[length + 1];
        memcpy(copy, value, length + 1);
        return copy;
    }

    ResourceCache* CreateResourceCache(void* userContext, FDestroyResource destroy)
    {
        ResourceCache* cache = new ResourceCache;
        cache->UserContext   = userContext;
        cache->Destroy       = destroy;
        return cache;
    }

    void DestroyResourceCache(ResourceCache* cache)
    {
        if (cache == nullptr)
        {
            return;
        }

        // Newest first: later resources are the ones most likely to reference
        // earlier ones through another cache.
        for (usize i = cache->Entries.Count(); i > 0; --i)
        {
            ResourceEntry& entry = cache->Entries[i - 1];
            cache->Destroy(cache->UserContext, entry.Resource);
            delete[] entry.Path;
        }

        cache->Entries.Clear();

        delete cache;
    }

    void* CacheFind(ResourceCache* cache, const char* path)
    {
        if (cache == nullptr || path == nullptr)
        {
            return nullptr;
        }

        const uint64 hash = utils::HashString(path);
        for (usize i = 0; i < cache->Entries.Count(); ++i)
        {
            ResourceEntry& entry = cache->Entries[i];
            if (entry.Hash == hash && strcmp(entry.Path, path) == 0)
            {
                ++entry.RefCount;
                return entry.Resource;
            }
        }

        return nullptr;
    }

    bool CacheInsert(ResourceCache* cache, const char* path, void* resource)
    {
        if (cache == nullptr || path == nullptr || resource == nullptr)
        {
            return false;
        }

        ResourceEntry entry = {
            .Resource = resource,
            .Path     = CopyString(path),
            .Hash     = utils::HashString(path),
            .RefCount = 1,
        };

        if (!cache->Entries.Add(entry))
        {
            CW_ERROR("Failed to cache resource %s", path);
            delete[] entry.Path;
            return false;
        }

        return true;
    }

    void CacheRelease(ResourceCache* cache, void* resource)
    {
        if (cache == nullptr || resource == nullptr)
        {
            return;
        }

        for (usize i = 0; i < cache->Entries.Count(); ++i)
        {
            ResourceEntry& entry = cache->Entries[i];
            if (entry.Resource != resource)
            {
                continue;
            }

            if (--entry.RefCount > 0)
            {
                return;
            }

            cache->Destroy(cache->UserContext, entry.Resource);
            delete[] entry.Path;
            cache->Entries.RemoveAt(i);
            return;
        }
    }
}
