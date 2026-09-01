#pragma once

#include "core/types.h"
#include "math/vector.h"
#include "math/matrix.h"

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
        VERTEX_FORMAT_UBYTE4,
    };

    struct Viewport
    {
        Vec2i Size;
    };

    struct VertexAttribute
    {
        const char*   Name;
        VERTEX_FORMAT Format;
    };

    typedef void* HPipeline;

    typedef void* HBuffer;

    typedef void* HUniform;

    struct Mesh
    {
        HBuffer Vertices;
        HBuffer Indicies;
    };

    enum UNIFORM_TYPE
    {
        UNIFORM_TYPE_FLOAT,
        UNIFORM_TYPE_VEC2,
        UNIFORM_TYPE_VEC3,
        UNIFORM_TYPE_VEC4,
        UNIFORM_TYPE_MAT4,
    };

    struct UniformDesc
    {
        UNIFORM_TYPE Type;
        const char*  Name;
    };

    struct PipelineDesc
    {
        const char*            VertexSource;
        const char*            FragmentSource;
        const VertexAttribute* Attributes;
        int                    AttributeCount;
        UniformDesc*           Uniforms;
        usize                  UniformsCount = 0;
    };

    struct BufferDesc
    {
        usize Count;
        usize Size;
        void*  Data;
    };

    typedef bool (*FInitialize)(void* window);

    typedef void (*FDestroy)();

    typedef void (*FBeginFrame)();

    typedef void (*FEndFrame)();

    typedef void (*FUpdateViewport)(int width, int height);

    typedef HPipeline (*FCreatePipeline)(const PipelineDesc* desc);

    typedef void (*FDestroyPipeline)(const HPipeline pipeline);

    typedef void (*FBindPipeline)(const HPipeline pipeline);

    typedef HBuffer (*FCreateBuffer)(const BufferDesc* desc);

    typedef void (*FDeleteBuffer)(const HBuffer buffer);

    typedef void (*FDrawMesh)(const Mesh* mesh, HPipeline pipeline);

    typedef void (*FGetUniform)(const HPipeline pipeline, uint64 name_hash, HUniform* out_uniform);

    typedef void (*FSetFloat)(const HUniform uniform, float value);

    typedef void (*FSetVec2)(const HUniform uniform, Vec2 value);

    typedef void (*FSetVec3)(const HUniform uniform, Vec3 value);

    typedef void (*FSetVec4)(const HUniform uniform, Vec4 value);

    typedef void (*FSetMat4)(const HUniform uniform, const float* value);

    struct RenderBackend
    {
        FInitialize         Initialize;
        FDestroy            Destroy;
        FBeginFrame         BeginFrame;
        FEndFrame           EndFrame;
        FUpdateViewport     UpdateViewport;
        FCreatePipeline     CreatePipeline;
        FDestroyPipeline    DestroyPipeline;
        FBindPipeline       BindPipeline;
        FGetUniform         GetUniform;
        FSetFloat           SetFloat;
        FSetVec2            SetVec2;
        FSetVec3            SetVec3;
        FSetVec4            SetVec4;
        FSetMat4            SetMat4;
        FCreateBuffer       CreateBuffer;
        FDeleteBuffer       DeleteBuffer;
        FDrawMesh           DrawMesh;

        RENDER_BACKEND_TYPE BackendType = RENDER_BACKEND_NONE;
    };
}
