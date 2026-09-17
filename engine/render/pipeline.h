#pragma once

#include "core/types.h"
#include "graphics/graphics_types.h"

namespace cw::render
{
    struct RenderContext;

    struct PipelineUniform
    {
        uint64                     NameHash = 0;
        char*                      Name     = nullptr;
        graphics::HUniformLocation Location = graphics::INVALID_UNIFORM_LOCATION;
        graphics::UNIFORM_TYPE     Type     = graphics::UNIFORM_TYPE_FLOAT;
    };

    enum BUILTIN_UNIFORMS
    {
        BUILTIN_UNIFORM_MODEL = 0,
        BUILTIN_UNIFORM_VIEW,
        BUILTIN_UNIFORM_PROJECTION,
        BUILTIN_UNIFORM_COUNT
    };

    // Builtins are kept in their own fixed array so that Uniforms stays strictly
    // parallel to Material::Parameters: index i in one addresses index i in the other.
    struct Pipeline
    {
        graphics::HPipeline        BackendPipeline = {};
        graphics::HUniformLocation Builtins[BUILTIN_UNIFORM_COUNT] = {};
        PipelineUniform*           Uniforms        = nullptr;
        usize                      UniformsCount   = 0;
    };

    struct PipelineUniformDesc
    {
        const char*            Name;
        graphics::UNIFORM_TYPE Type;
    };

    // Everything the render layer needs to build a pipeline, already parsed.
    // DebugName is only ever used to make error messages traceable.
    struct PipelineCreateDesc
    {
        const char*                      DebugName      = nullptr;
        const char*                      VertexSource   = nullptr;
        const char*                      FragmentSource = nullptr;
        const graphics::VertexAttribute* Attributes     = nullptr;
        usize                            AttributeCount = 0;
        const PipelineUniformDesc*       Uniforms       = nullptr;
        usize                            UniformCount   = 0;
    };

    Pipeline* CreatePipeline(RenderContext* ctx, const PipelineCreateDesc* desc);

    void DestroyPipeline(RenderContext* ctx, Pipeline* pipeline);

    bool FindPipelineUniform(const Pipeline* pipeline, const char* name, usize* outIndex);

    usize GetUniformValueCount(graphics::UNIFORM_TYPE type);
}
