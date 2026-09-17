#include "material.h"

#include "logger/log.h"
#include "render_private.h"
#include "utils/hash.h"

#include <cstring>
#include <type_traits>

namespace cw::render
{
    Material* CreateMaterial(
        RenderContext*           ctx,
        const Pipeline*          pipeline,
        const MaterialParamDesc* params,
        usize                    paramCount,
        const char*              debugName
    )
    {
        if (ctx == nullptr || pipeline == nullptr)
        {
            return nullptr;
        }

        Material* material   = new Material{};
        material->Pipeline   = pipeline;
        material->Parameters = new MaterialParameter[pipeline->UniformsCount]{};

        for (usize i = 0; i < paramCount; ++i)
        {
            const MaterialParamDesc& param = params[i];

            usize uniformIndex = 0;
            if (!FindPipelineUniform(pipeline, param.Name, &uniformIndex))
            {
                CW_ERROR(
                    "%s: parameter \"%s\" is not declared by its pipeline",
                    debugName,
                    param.Name
                );
                DestroyMaterial(material);
                return nullptr;
            }

            const usize expectedCount =
                GetUniformValueCount(pipeline->Uniforms[uniformIndex].Type);
            if (param.ValueCount != expectedCount)
            {
                CW_ERROR(
                    "%s: parameter \"%s\" has %zu values, expected %zu",
                    debugName,
                    param.Name,
                    param.ValueCount,
                    expectedCount
                );
                DestroyMaterial(material);
                return nullptr;
            }

            memcpy(
                material->Parameters[uniformIndex].Elements[0].Data,
                param.Values,
                param.ValueCount * sizeof(float)
            );
        }

        return material;
    }

    void DestroyMaterial(Material* material)
    {
        if (material == nullptr)
        {
            return;
        }

        delete[] material->Parameters;
        delete material;
    }

    static MaterialParameter* FindParameter(
        Material*              material,
        uint64                 nameHash,
        graphics::UNIFORM_TYPE expectedType
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
        Material*              material,
        uint64                 nameHash,
        graphics::UNIFORM_TYPE expectedType,
        T                      value
    )
    {
        MaterialParameter* parameter = FindParameter(material, nameHash, expectedType);
        if (parameter == nullptr)
        {
            return;
        }

        std::memcpy(parameter->Elements[0].Data, &value, sizeof(value));
    }

    void SetFloat(Material* material, uint64 nameHash, float value)
    {
        SetUniformValue(material, nameHash, graphics::UNIFORM_TYPE_FLOAT, value);
    }

    void SetVec2(Material* material, uint64 nameHash, Vec2 value)
    {
        SetUniformValue(material, nameHash, graphics::UNIFORM_TYPE_VEC2, value);
    }

    void SetVec3(Material* material, uint64 nameHash, Vec3 value)
    {
        SetUniformValue(material, nameHash, graphics::UNIFORM_TYPE_VEC3, value);
    }

    void SetVec4(Material* material, uint64 nameHash, Vec4 value)
    {
        SetUniformValue(material, nameHash, graphics::UNIFORM_TYPE_VEC4, value);
    }

    void SetMat4(Material* material, uint64 nameHash, Mat4 value)
    {
        SetUniformValue(material, nameHash, graphics::UNIFORM_TYPE_MAT4, value);
    }
}
