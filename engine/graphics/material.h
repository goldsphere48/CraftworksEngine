#pragma once

#include "pipeline_manager.h"
#include "renderer_backend.h"

namespace cw::graphics
{
    struct MaterialParameter
    {
        float Value[16]{};
    };

    struct Material
    {
        const PipelineResource* Pipeline   = nullptr;
        MaterialParameter*      Parameters = nullptr;
    };

    struct MaterialContext;

    MaterialContext* CreateContext(PipelineManager* pipelineManger);

    void DestroyContext(MaterialContext* context);

    Material* LoadMaterial(const MaterialContext* context, const char* path);

    void DestroyMaterial(const Material* material);

    void SetFloat(Material* material, uint64 nameHash, float value);

    void SetVec2(Material* material, uint64 nameHash, Vec2 value);

    void SetVec3(Material* material, uint64 nameHash, Vec3 value);

    void SetVec4(Material* material, uint64 nameHash, Vec4 value);

    void SetMat4(Material* material, uint64 nameHash, Mat4 value);
}
