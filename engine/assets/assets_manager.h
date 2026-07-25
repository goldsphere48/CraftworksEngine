#pragma once

#include "filesystem/filesystem.h"
#include "core/types.h"

namespace cw::assets
{
    struct PipelineAsset
    {
        void*           Binary         = nullptr;
        usize           BinarySize     = 0;
        fs::FileBuffer* VertexBuffer   = nullptr;
        fs::FileBuffer* FragmentBuffer = nullptr;
    };

    PipelineAsset* LoadPipelineAsset(const char* path);

    void FreePipelineAsset(PipelineAsset* asset);
}
