#pragma once

#include "pipeline_manager.h"
#include "renderer_backend.h"

namespace cw::material
{
    struct MaterialParameter
    {
        float Value[16]{};
    };

    struct Material
    {
        const graphics::PipelineResource* Pipeline   = nullptr;
        MaterialParameter*                Parameters = nullptr;
    };

    struct MaterialContext;

    MaterialContext* CreateContext(graphics::PipelineManager* pipelineManger);

    void DestroyContext(MaterialContext* context);

    Material* LoadMaterial(const MaterialContext* context, const char* path);

    void DestroyMaterial(const Material* material);

    void SetFloat(Material* material, uint64 name_hash, float value);

    void SetVec2(Material* material, uint64 name_hash, Vec2 value);

    void SetVec3(Material* material, uint64 name_hash, Vec3 value);

    void SetVec4(Material* material, uint64 name_hash, Vec4 value);

    void SetMat4(Material* material, uint64 name_hash, Mat4 value);
}
