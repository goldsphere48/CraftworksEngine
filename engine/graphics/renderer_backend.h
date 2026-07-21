#pragma once

#include <stddef.h>

namespace cw::graphics
{
    enum RENDER_BACKEND_TYPE
    {
        RENDER_BACKEND_NONE,
        RENDER_BACKEND_OPENGL,
    };

    enum SHADER_TYPE
    {
        SHADER_TYPE_VERTEX,
        SHADER_TYPE_FRAGMENT,
    };

    enum VERTEX_FORMAT
    {
        VERTEX_FORMAT_FLOAT,
        VERTEX_FORMAT_FLOAT2,
        VERTEX_FORMAT_FLOAT3,
        VERTEX_FORMAT_FLOAT4,
        VERTEX_FORMAT_UBYTE4_NORM,
        VERTEX_FORMAT_UBYTE4,
    };

    struct VertexAttribute
    {
        int           Location;
        VERTEX_FORMAT Format;
    };

    struct VertexLayout
    {
        const VertexAttribute* Attributes;
        int                    AttributeCount;
        int                    Stride;
    };

    typedef void* HPipeline;

    struct ShaderDesc
    {
        const char* VertexSource;
        const char* FragmentSource;
    };

    struct PipelineDesc
    {
        const ShaderDesc  Source;
        const void*       Binary;
        size_t            BinarySize;
    };

    typedef bool (*FInitialize)(void* window);

    typedef void (*FDestroy)();

    typedef void (*FBeginFrame)();

    typedef void (*FEndFrame)();

    typedef HPipeline (*FCreatePipeline)(const PipelineDesc* desc);

    typedef void (*FDestroyPipeline)(HPipeline pipeline);

    typedef void (*FBindPipeline)(HPipeline pipeline);

    typedef void (*FGetPipelineBinary)(HPipeline pipeline, void** out_data, size_t* out_size);

    typedef const char* (*FGetPipelineCacheId)();

    struct RenderBackend
    {
        FInitialize         Initialize;
        FDestroy            Destroy;
        FBeginFrame         BeginFrame;
        FEndFrame           EndFrame;
        FCreatePipeline     CreatePipeline;
        FDestroyPipeline    DestroyPipeline;
        FBindPipeline       BindPipeline;
        FGetPipelineBinary  GetPipelineBinary;
        FGetPipelineCacheId GetPipelineCacheId;

        RENDER_BACKEND_TYPE BackendType = RENDER_BACKEND_NONE;
    };
}
