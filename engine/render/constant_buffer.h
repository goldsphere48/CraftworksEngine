#pragma once

#include "core/types.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "pipeline.h"

namespace cw::render
{
    struct RenderContext;
    struct ConstantBuffer;

    // Per-instance overrides of a material's values, addressed by name hash. The
    // storage is taken once at construction: setting a constant never allocates.
    constexpr usize CONSTANT_MAX_ELEMENTS = 4;

    ConstantBuffer* CreateConstantBuffer(usize maxConstants);

    void DestroyConstantBuffer(ConstantBuffer* cb);

    void SetConstant(ConstantBuffer* cb, uint64 nameHash, const Vec4* values, usize count);

    void SetConstantMat4(ConstantBuffer* cb, uint64 nameHash, const Mat4* value);

    bool GetConstant(
        const ConstantBuffer* cb,
        uint64                nameHash,
        const Vec4**          outValues,
        usize*                outCount
    );

    void ClearConstants(ConstantBuffer* cb);

    // Writes every constant this buffer holds into the currently bound pipeline.
    void ApplyConstants(RenderContext* ctx, const Pipeline* pipeline, const ConstantBuffer* cb);
}
