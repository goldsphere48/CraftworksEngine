#pragma once

#include "core/types.h"
#include "filesystem/filesystem.h"
#include "graphics/renderer_backend.h"

namespace cw::assets
{
    struct PipelineAsset
    {
        struct VertexParameter
        {
            const char*                   Name;
            const graphics::VERTEX_FORMAT Format;
        };

        struct UniformParameter
        {
            const char*                  Name;
            const graphics::UNIFORM_TYPE Type;
        };

        fs::FileBuffer*   VertexBuffer            = nullptr;
        fs::FileBuffer*   FragmentBuffer          = nullptr;
        VertexParameter*  VerticesAttributesInfo  = nullptr;
        usize             VerticesAttributesCount = 0;
        UniformParameter* UniformsInfo            = nullptr;
        usize             UniformsCount           = 0;
    };

    struct MaterialAsset
    {
        struct Parameter
        {
            const char* Name;
            float*      Value;
            usize       ValueCount;
        };

        const char* PipelineAssetPath;
        Parameter*  Parameters;
        usize       ParametersCount;
    };

    PipelineAsset* LoadPipelineAsset(const char* path);

    MaterialAsset* LoadMaterialAsset(const char* path);

    void FreePipelineAsset(PipelineAsset* asset);

    void FreeMaterialAsset(MaterialAsset* asset);
}
