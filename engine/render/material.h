#pragma once

#include "math/matrix.h"
#include "math/vector.h"
#include "pipeline.h"

namespace cw::render
{
    struct RenderContext;

    // Values are stored promoted to whole vec4 elements: one for anything up to a
    // vec4, four for a mat4. That is the shape the backend writes in, and the shape
    // a uniform buffer would want.
    constexpr usize MATERIAL_PARAMETER_MAX_ELEMENTS = 4;

    struct MaterialParameter
    {
        Vec4 Elements[MATERIAL_PARAMETER_MAX_ELEMENTS]{};
    };

    struct Material
    {
        const Pipeline*    Pipeline   = nullptr;
        MaterialParameter* Parameters = nullptr;
    };

    struct MaterialParamDesc
    {
        const char*  Name       = nullptr;
        const float* Values     = nullptr;
        usize        ValueCount = 0;
    };

    Material* CreateMaterial(
        RenderContext*           ctx,
        const Pipeline*          pipeline,
        const MaterialParamDesc* params,
        usize                    paramCount,
        const char*              debugName
    );

    void DestroyMaterial(Material* material);

    void SetFloat(Material* material, uint64 nameHash, float value);

    void SetVec2(Material* material, uint64 nameHash, Vec2 value);

    void SetVec3(Material* material, uint64 nameHash, Vec3 value);

    void SetVec4(Material* material, uint64 nameHash, Vec4 value);

    void SetMat4(Material* material, uint64 nameHash, Mat4 value);
}
