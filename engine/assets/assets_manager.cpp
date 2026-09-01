#include "assets_manager.h"

#include "logger/log.h"
#include "tomlc17/tomlc17.h"

#include <cstring>
#include <new>

namespace cw::assets
{
    static bool ParseVertexFormat(const char* format, graphics::VERTEX_FORMAT* out_format)
    {
        if (strcmp(format, "float") == 0) { *out_format = graphics::VERTEX_FORMAT_FLOAT;  return true; }
        if (strcmp(format, "vec2")  == 0) { *out_format = graphics::VERTEX_FORMAT_FLOAT2; return true; }
        if (strcmp(format, "vec3")  == 0) { *out_format = graphics::VERTEX_FORMAT_FLOAT3; return true; }
        if (strcmp(format, "vec4")  == 0) { *out_format = graphics::VERTEX_FORMAT_FLOAT4; return true; }
        if (strcmp(format, "ubyte4") == 0) { *out_format = graphics::VERTEX_FORMAT_UBYTE4; return true; }
        return false;
    }

    static bool ParseUniformType(const char* format, graphics::UNIFORM_TYPE* out_type)
    {
        if (strcmp(format, "float") == 0) { *out_type = graphics::UNIFORM_TYPE_FLOAT; return true; }
        if (strcmp(format, "vec2")  == 0) { *out_type = graphics::UNIFORM_TYPE_VEC2;  return true; }
        if (strcmp(format, "vec3")  == 0) { *out_type = graphics::UNIFORM_TYPE_VEC3;  return true; }
        if (strcmp(format, "vec4")  == 0) { *out_type = graphics::UNIFORM_TYPE_VEC4;  return true; }
        if (strcmp(format, "mat4")  == 0) { *out_type = graphics::UNIFORM_TYPE_MAT4;  return true; }
        return false;
    }

    static char* CopyString(const char* ptr, int len)
    {
        char* buffer = new char[len + 1];
        memcpy(buffer, ptr, len);
        buffer[len] = '\0';
        return buffer;
    }

    static void FreeVertexAttributes(PipelineAsset::VertexParameter* info, usize count)
    {
        for (usize i = 0; i < count; ++i)
        {
            delete[] info[i].Name;
        }
        free(info);
    }

    static void FreeUniformParameters(PipelineAsset::UniformParameter* info, usize count)
    {
        for (usize i = 0; i < count; ++i)
        {
            delete[] info[i].Name;
        }
        free(info);
    }

    static bool ParseVertexAttributes(toml_datum_t arr, const char* path, PipelineAsset::VertexParameter** out_info, usize* out_count)
    {
        PipelineAsset::VertexParameter* info = (PipelineAsset::VertexParameter*)malloc(sizeof(PipelineAsset::VertexParameter) * arr.u.arr.size);

        for (int i = 0; i < arr.u.arr.size; ++i)
        {
            toml_datum_t entry = arr.u.arr.elem[i];
            if (entry.type != TOML_TABLE)
            {
                CW_ERROR("%s: [[vertexattribute]] entry #%d is not a table", path, i + 1);
                FreeVertexAttributes(info, i);
                return false;
            }

            toml_datum_t name   = toml_get(entry, "name");
            toml_datum_t format = toml_get(entry, "format");

            if (name.type != TOML_STRING)
            {
                CW_ERROR("%s:%d: [[vertexattribute]] entry #%d is missing a quoted \"name\" field", path, entry.lineno, i + 1);
                FreeVertexAttributes(info, i);
                return false;
            }

            if (format.type != TOML_STRING)
            {
                CW_ERROR("%s:%d: [[vertexattribute]] \"%.*s\" is missing a quoted \"format\" field", path, entry.lineno, name.u.str.len, name.u.str.ptr);
                FreeVertexAttributes(info, i);
                return false;
            }

            graphics::VERTEX_FORMAT vertexFormat;
            if (!ParseVertexFormat(format.u.str.ptr, &vertexFormat))
            {
                CW_ERROR("%s:%d: [[vertexattribute]] \"%.*s\" has unknown format \"%.*s\"", path, entry.lineno, name.u.str.len, name.u.str.ptr, format.u.str.len, format.u.str.ptr);
                FreeVertexAttributes(info, i);
                return false;
            }

            new (&info[i]) PipelineAsset::VertexParameter{ CopyString(name.u.str.ptr, name.u.str.len), vertexFormat };
        }

        *out_info  = info;
        *out_count = (usize)arr.u.arr.size;
        return true;
    }

    static bool ParseUniformParameters(toml_datum_t arr, const char* path, PipelineAsset::UniformParameter** out_info, usize* out_count)
    {
        PipelineAsset::UniformParameter* info = (PipelineAsset::UniformParameter*)malloc(sizeof(PipelineAsset::UniformParameter) * arr.u.arr.size);

        for (int i = 0; i < arr.u.arr.size; ++i)
        {
            toml_datum_t entry = arr.u.arr.elem[i];
            if (entry.type != TOML_TABLE)
            {
                CW_ERROR("%s: [[uniform]] entry #%d is not a table", path, i + 1);
                FreeUniformParameters(info, i);
                return false;
            }

            toml_datum_t name   = toml_get(entry, "name");
            toml_datum_t format = toml_get(entry, "format");

            if (name.type != TOML_STRING)
            {
                CW_ERROR("%s:%d: [[uniform]] entry #%d is missing a quoted \"name\" field", path, entry.lineno, i + 1);
                FreeUniformParameters(info, i);
                return false;
            }

            if (format.type != TOML_STRING)
            {
                CW_ERROR("%s:%d: [[uniform]] \"%.*s\" is missing a quoted \"format\" field", path, entry.lineno, name.u.str.len, name.u.str.ptr);
                FreeUniformParameters(info, i);
                return false;
            }

            graphics::UNIFORM_TYPE uniformType;
            if (!ParseUniformType(format.u.str.ptr, &uniformType))
            {
                CW_ERROR("%s:%d: [[uniform]] \"%.*s\" has unknown format \"%.*s\"", path, entry.lineno, name.u.str.len, name.u.str.ptr, format.u.str.len, format.u.str.ptr);
                FreeUniformParameters(info, i);
                return false;
            }

            new (&info[i]) PipelineAsset::UniformParameter{ CopyString(name.u.str.ptr, name.u.str.len), uniformType };
        }

        *out_info  = info;
        *out_count = (usize)arr.u.arr.size;
        return true;
    }

    PipelineAsset* LoadPipelineAsset(const char* path)
    {
        fs::FileBuffer* src = fs::ReadFile(path);
        if (src == nullptr)
        {
            CW_ERROR("Pipeline file not found: %s", path);
            return nullptr;
        }

        toml_result_t result = toml_parse((const char*)src->Data, (int)src->Size);

        fs::FreeFile(src);

        if (!result.ok)
        {
            CW_ERROR("Failed to parse pipeline %s: %s", path, result.errmsg);
            return nullptr;
        }

        toml_datum_t vertexPath   = toml_seek(result.toptab, "vertex");
        toml_datum_t fragmentPath = toml_seek(result.toptab, "fragment");
        if (vertexPath.type != TOML_STRING)
        {
            CW_ERROR("%s: missing required string field \"vertex\" (path to the vertex shader source)", path);
            toml_free(result);
            return nullptr;
        }
        if (fragmentPath.type != TOML_STRING)
        {
            CW_ERROR("%s: missing required string field \"fragment\" (path to the fragment shader source)", path);
            toml_free(result);
            return nullptr;
        }

        toml_datum_t attributesArr = toml_seek(result.toptab, "vertexattribute");
        toml_datum_t uniformsArr   = toml_seek(result.toptab, "uniform");
        if (attributesArr.type != TOML_ARRAY)
        {
            CW_ERROR("%s: no vertex attributes declared, expected at least one [[vertexattribute]] table", path);
            toml_free(result);
            return nullptr;
        }
        if (uniformsArr.type != TOML_ARRAY)
        {
            CW_ERROR("%s: no uniforms declared, expected at least one [[uniform]] table", path);
            toml_free(result);
            return nullptr;
        }

        fs::FileBuffer* vertexSource = fs::ReadFile(vertexPath.u.s);
        if (!vertexSource)
        {
            CW_ERROR("%s: failed to read vertex shader source \"%s\"", path, vertexPath.u.s);
            toml_free(result);
            return nullptr;
        }

        fs::FileBuffer* fragmentSource = fs::ReadFile(fragmentPath.u.s);
        if (!fragmentSource)
        {
            CW_ERROR("%s: failed to read fragment shader source \"%s\"", path, fragmentPath.u.s);
            fs::FreeFile(vertexSource);
            toml_free(result);
            return nullptr;
        }

        PipelineAsset::VertexParameter* attributes      = nullptr;
        usize                           attributesCount = 0;
        if (!ParseVertexAttributes(attributesArr, path, &attributes, &attributesCount))
        {
            fs::FreeFile(vertexSource);
            fs::FreeFile(fragmentSource);
            toml_free(result);
            return nullptr;
        }

        PipelineAsset::UniformParameter* uniforms      = nullptr;
        usize                            uniformsCount = 0;
        if (!ParseUniformParameters(uniformsArr, path, &uniforms, &uniformsCount))
        {
            FreeVertexAttributes(attributes, attributesCount);
            fs::FreeFile(vertexSource);
            fs::FreeFile(fragmentSource);
            toml_free(result);
            return nullptr;
        }

        toml_free(result);

        PipelineAsset* asset           = new PipelineAsset;
        asset->VertexBuffer            = vertexSource;
        asset->FragmentBuffer          = fragmentSource;
        asset->VerticesAttributesInfo  = attributes;
        asset->VerticesAttributesCount = attributesCount;
        asset->UniformsInfo            = uniforms;
        asset->UniformsCount           = uniformsCount;

        return asset;
    }

    void FreePipelineAsset(PipelineAsset* asset)
    {
        fs::FreeFile(asset->VertexBuffer);
        fs::FreeFile(asset->FragmentBuffer);

        FreeVertexAttributes(asset->VerticesAttributesInfo, asset->VerticesAttributesCount);
        FreeUniformParameters(asset->UniformsInfo, asset->UniformsCount);

        delete asset;
    }

    static void FreeMaterialParameters(MaterialAsset::Parameter* parameters, usize count)
    {
        for (usize i = 0; i < count; ++i)
        {
            delete[] parameters[i].Name;
            delete[] parameters[i].Value;
        }
        delete[] parameters;
    }

    static bool ParseMaterialParameters(toml_datum_t arr, const char* path, MaterialAsset::Parameter** out_parameters, usize* out_count)
    {
        MaterialAsset::Parameter* parameters = new MaterialAsset::Parameter[arr.u.arr.size];

        for (int i = 0; i < arr.u.arr.size; ++i)
        {
            toml_datum_t entry = arr.u.arr.elem[i];
            if (entry.type != TOML_TABLE)
            {
                CW_ERROR("%s: [[parameter]] entry #%d is not a table", path, i + 1);
                FreeMaterialParameters(parameters, i);
                return false;
            }

            toml_datum_t name  = toml_get(entry, "name");
            toml_datum_t value = toml_get(entry, "value");

            if (name.type != TOML_STRING)
            {
                CW_ERROR("%s:%d: [[parameter]] entry #%d is missing a quoted \"name\" field", path, entry.lineno, i + 1);
                FreeMaterialParameters(parameters, i);
                return false;
            }

            if (value.type != TOML_ARRAY || value.u.arr.size == 0)
            {
                CW_ERROR("%s:%d: parameter \"%.*s\" is missing a non-empty \"value\" array", path, entry.lineno, name.u.str.len, name.u.str.ptr);
                FreeMaterialParameters(parameters, i);
                return false;
            }

            float* values = new float[value.u.arr.size];

            for (int v = 0; v < value.u.arr.size; ++v)
            {
                toml_datum_t component = value.u.arr.elem[v];
                if (component.type == TOML_INT64)
                {
                    values[v] = (float)component.u.int64;
                }
                else if (component.type == TOML_FP64)
                {
                    values[v] = (float)component.u.fp64;
                }
                else
                {
                    CW_ERROR("%s:%d: parameter \"%.*s\" value[%d] is not a number", path, entry.lineno, name.u.str.len, name.u.str.ptr, v);
                    delete[] values;
                    FreeMaterialParameters(parameters, i);
                    return false;
                }
            }

            parameters[i].Name       = CopyString(name.u.str.ptr, name.u.str.len);
            parameters[i].Value      = values;
            parameters[i].ValueCount = (usize)value.u.arr.size;
        }

        *out_parameters = parameters;
        *out_count      = (usize)arr.u.arr.size;
        return true;
    }

    MaterialAsset* LoadMaterialAsset(const char* path)
    {
        fs::FileBuffer* src = fs::ReadFile(path);
        if (src == nullptr)
        {
            CW_ERROR("Material file not found: %s", path);
            return nullptr;
        }

        toml_result_t result = toml_parse((const char*)src->Data, (int)src->Size);

        fs::FreeFile(src);

        if (!result.ok)
        {
            CW_ERROR("Failed to parse material %s: %s", path, result.errmsg);
            return nullptr;
        }

        toml_datum_t pipelinePath = toml_seek(result.toptab, "pipeline");
        if (pipelinePath.type != TOML_STRING)
        {
            CW_ERROR("%s: missing required string field \"pipeline\" (path to the pipeline asset)", path);
            toml_free(result);
            return nullptr;
        }

        toml_datum_t parametersArr = toml_seek(result.toptab, "parameter");
        if (parametersArr.type != TOML_ARRAY)
        {
            CW_ERROR("%s: no parameters declared, expected at least one [[parameter]] table", path);
            toml_free(result);
            return nullptr;
        }

        char* pipelinePathCopy = CopyString(pipelinePath.u.str.ptr, pipelinePath.u.str.len);

        MaterialAsset::Parameter* parameters      = nullptr;
        usize                     parametersCount = 0;
        if (!ParseMaterialParameters(parametersArr, path, &parameters, &parametersCount))
        {
            delete[] pipelinePathCopy;
            toml_free(result);
            return nullptr;
        }

        toml_free(result);

        MaterialAsset* asset      = new MaterialAsset;
        asset->PipelineAssetPath  = pipelinePathCopy;
        asset->Parameters         = parameters;
        asset->ParametersCount    = parametersCount;

        return asset;
    }

    void FreeMaterialAsset(MaterialAsset* asset)
    {
        delete[] asset->PipelineAssetPath;
        FreeMaterialParameters(asset->Parameters, asset->ParametersCount);
        delete asset;
    }
}
