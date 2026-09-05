#pragma once

#include "renderer_backend.h"

namespace cw::graphics
{
    struct GraphicsContext;
    struct PipelineManager;

    struct PipelineUniform
    {
        uint64       NameHash       = 0;
        const char*  Name           = nullptr;
        HUniform     BackendUniform = {};
        UNIFORM_TYPE Type           = UNIFORM_TYPE_FLOAT;
    };

    struct PipelineResource
    {
        HPipeline        BackendPipeline = {};
        PipelineUniform* Uniforms        = nullptr;
        usize            UniformsCount   = 0;
    };

    PipelineManager* CreatePipelineManager(GraphicsContext* ctx);

    void DestroyPipelineManager(PipelineManager* manager);

    const PipelineResource* AcquirePipeline(PipelineManager* manager, const char* path);
}
