#include "assets_manager.h"

#include "logger/log.h"
#include "tomlc17/tomlc17.h"

namespace cw::assets
{
    PipelineAsset* LoadPipelineAsset(const char* path)
    {
        fs::FileBuffer* src = fs::ReadFile(path);
        if (src == nullptr)
        {
            CW_ERROR("Failed to find pipeline source %s", path);
            return nullptr;
        }

        toml_result_t result = toml_parse((const char*)src->Data, src->Size);

        fs::FreeFile(src);

        if (!result.ok)
        {
            CW_ERROR("Failed to parse pipeline source %s. Error: %s", path, result.errmsg);
            return nullptr;
        }

        toml_datum_t vertex   = toml_seek(result.toptab, "vertex");
        toml_datum_t fragment = toml_seek(result.toptab, "fragment");

        if (vertex.type != TOML_STRING || fragment.type != TOML_STRING)
        {
            toml_free(result);
            CW_ERROR("Failed to get shader source property %s", path);
            return nullptr;
        }

        fs::FileBuffer* vertexSource = fs::ReadFile(vertex.u.s);
        if (!vertexSource)
        {
            toml_free(result);
            CW_ERROR("Failed to read vertex sources %s", path);
            return nullptr;
        }

        fs::FileBuffer* fragmentSource = fs::ReadFile(fragment.u.s);
        if (!fragmentSource)
        {
            toml_free(result);
            fs::FreeFile(vertexSource);
            CW_ERROR("Failed to read fragment sources %s", path);
            return nullptr;
        }

        PipelineAsset* asset  = new PipelineAsset;
        asset->VertexBuffer   = vertexSource;
        asset->FragmentBuffer = fragmentSource;

        toml_free(result);

        return asset;
    }

    void FreePipelineAsset(PipelineAsset* asset)
    {
        fs::FreeFile(asset->VertexBuffer);
        fs::FreeFile(asset->FragmentBuffer);

        delete asset;
    }
}
