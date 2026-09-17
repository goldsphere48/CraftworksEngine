#pragma once

#include "core/types.h"

// Path-keyed, reference-counted store of engine resources. The cache never
// interprets what it holds: creation belongs to the caller, destruction to the
// callback handed in at construction.

namespace cw::assets
{
    struct ResourceCache;

    typedef void (*FDestroyResource)(void* userContext, void* resource);

    ResourceCache* CreateResourceCache(void* userContext, FDestroyResource destroy);

    // Destroys every resource still held, newest first.
    void DestroyResourceCache(ResourceCache* cache);

    // Returns the cached resource and takes one reference on it, or nullptr.
    void* CacheFind(ResourceCache* cache, const char* path);

    // Takes ownership of the resource with a reference count of one.
    bool CacheInsert(ResourceCache* cache, const char* path, void* resource);

    // Drops one reference; destroys the resource when the last one goes.
    void CacheRelease(ResourceCache* cache, void* resource);
}
