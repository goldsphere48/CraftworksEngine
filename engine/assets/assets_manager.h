#pragma once

#include "filesystem/filesystem.h"

#include <stddef.h>

namespace cw::assets
{
    struct PipelineAsset
    {
        void*           Binary         = nullptr;
        size_t          BinarySize     = 0;
        fs::FileBuffer* VertexBuffer   = nullptr;
        fs::FileBuffer* FragmentBuffer = nullptr;
    };

    PipelineAsset* LoadPipelineAsset(const char* path);

    void FreePipelineAsset(PipelineAsset* asset);
}
