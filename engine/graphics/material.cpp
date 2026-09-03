#include "material.h"

#include "assets/assets_manager.h"
#include "logger/log.h"
#include "utils/hash.h"

#include <cstring>
#include <type_traits>

namespace cw::graphics
{
    struct MaterialContext
    {
        PipelineManager* PipelineManager;
    };

    MaterialContext* CreateContext(PipelineManager* pipelineManager)
    {
        MaterialContext* context = new MaterialContext;
        context->PipelineManager = pipelineManager;
        return context;
    }

    void DestroyContext(MaterialContext* context)
    {
        delete context;
    }
    
    static usize GetUniformValueCount(UNIFORM_TYPE type)
    {
        switch (type)
        {
            case UNIFORM_TYPE_FLOAT:
                return 1;
            case UNIFORM_TYPE_VEC2:
                return 2;
            case UNIFORM_TYPE_VEC3:
                return 3;
            case UNIFORM_TYPE_VEC4:
                return 4;
            case UNIFORM_TYPE_MAT4:
                return 16;
        }

        return 0;
    }

    static bool FindPipelineUniform(
        const PipelineResource* pipeline,
        const char*             name,
        usize*                  outIndex
    )
    {
        const uint64 nameHash = utils::HashString(name);
        for (usize i = 0; i < pipeline->UniformsCount; ++i)
        {
            const PipelineUniform& uniform = pipeline->Uniforms[i];
            if (uniform.NameHash == nameHash && strcmp(uniform.Name, name) == 0)
            {
                *outIndex = i;
                return true;
            }
        }

        return false;
    }

    Material* LoadMaterial(const MaterialContext* context, const char* path)
    {
        if (context == nullptr || path == nullptr)
        {
            return nullptr;
        }

        assets::MaterialAsset* asset = assets::LoadMaterialAsset(path);
        if (asset == nullptr)
        {
            return nullptr;
        }

        const PipelineResource* pipeline =
            AcquirePipeline(context->PipelineManager, asset->PipelineAssetPath);
        if (pipeline == nullptr)
        {
            assets::FreeMaterialAsset(asset);
            return nullptr;
        }

        Material* material = new Material{};
        material->Pipeline = pipeline;
        material->Parameters = new MaterialParameter[pipeline->UniformsCount]{};

        for (usize i = 0; i < asset->ParametersCount; ++i)
        {
            const assets::MaterialAsset::Parameter& parameter = asset->Parameters[i];

            usize uniformIndex = 0;
            if (!FindPipelineUniform(pipeline, parameter.Name, &uniformIndex))
            {
                CW_ERROR("%s: parameter \"%s\" is not declared by its pipeline", path, parameter.Name);
                DestroyMaterial(material);
                assets::FreeMaterialAsset(asset);
                return nullptr;
            }

            const usize expectedCount = GetUniformValueCount(pipeline->Uniforms[uniformIndex].Type);
            if (parameter.ValueCount != expectedCount)
            {
                CW_ERROR(
                    "%s: parameter \"%s\" has %zu values, expected %zu",
                    path,
                    parameter.Name,
                    parameter.ValueCount,
                    expectedCount
                );
                DestroyMaterial(material);
                assets::FreeMaterialAsset(asset);
                return nullptr;
            }

            memcpy(
                material->Parameters[uniformIndex].Value,
                parameter.Value,
                parameter.ValueCount * sizeof(float)
            );
        }

        assets::FreeMaterialAsset(asset);

        return material;
    }

    void DestroyMaterial(const Material* material)
    {
        if (material == nullptr)
        {
            return;
        }

        delete[] material->Parameters;
        delete material;
    }

    static MaterialParameter* FindParameter(
        Material*    material,
        uint64       nameHash,
        UNIFORM_TYPE expectedType
    )
    {
        if (material == nullptr || material->Pipeline == nullptr)
        {
            return nullptr;
        }

        for (usize i = 0; i < material->Pipeline->UniformsCount; ++i)
        {
            const PipelineUniform& uniform = material->Pipeline->Uniforms[i];
            if (uniform.NameHash == nameHash && uniform.Type == expectedType)
            {
                return &material->Parameters[i];
            }
        }

        return nullptr;
    }

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    static void SetUniformValue(
        Material*    material,
        uint64       nameHash,
        UNIFORM_TYPE expectedType,
        T            value
    )
    {
        MaterialParameter* parameter = FindParameter(material, nameHash, expectedType);
        if (parameter == nullptr)
        {
            return;
        }

        std::memcpy(parameter->Value, &value, sizeof(value));
    }

    void SetFloat(Material* material, uint64 nameHash, float value)
    {
        SetUniformValue(material, nameHash, UNIFORM_TYPE_FLOAT, value);
    }

    void SetVec2(Material* material, uint64 nameHash, Vec2 value)
    {
        SetUniformValue(material, nameHash, UNIFORM_TYPE_VEC2, value);
    }

    void SetVec3(Material* material, uint64 nameHash, Vec3 value)
    {
        SetUniformValue(material, nameHash, UNIFORM_TYPE_VEC3, value);
    }

    void SetVec4(Material* material, uint64 nameHash, Vec4 value)
    {
        SetUniformValue(material, nameHash, UNIFORM_TYPE_VEC4, value);
    }

    void SetMat4(Material* material, uint64 nameHash, Mat4 value)
    {
        SetUniformValue(material, nameHash, UNIFORM_TYPE_MAT4, value);
    }
}
