#pragma once

#include "core/types.h"
#include "math/vector.h"

// Backend-neutral description of graphics resources. Everything here is safe to
// include from layers above the device (render, assets): it carries no backend
// entry points and no knowledge of how a resource is realised.

namespace cw::graphics
{
    enum ADAPTER_FAMILY
    {
        ADAPTER_FAMILY_NONE,
        ADAPTER_FAMILY_NULL,
        ADAPTER_FAMILY_OPENGL,
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

    enum UNIFORM_TYPE
    {
        UNIFORM_TYPE_FLOAT,
        UNIFORM_TYPE_VEC2,
        UNIFORM_TYPE_VEC3,
        UNIFORM_TYPE_VEC4,
        UNIFORM_TYPE_MAT4,
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

    struct HPipeline
    {
        void* Id = nullptr;
    };

    struct HBuffer
    {
        void* Id = nullptr;
    };

    template<typename THandle>
    constexpr bool IsValid(THandle handle)
    {
        return handle.Id != nullptr;
    }

    // Opaque, backend-assigned address of a uniform inside a pipeline. OpenGL
    // resolves it to a glUniform location, a Vulkan backend to an offset inside a
    // uniform buffer -- neither meaning escapes the backend.
    using HUniformLocation = int32;

    constexpr HUniformLocation INVALID_UNIFORM_LOCATION = -1;

    constexpr bool IsValidLocation(HUniformLocation location)
    {
        return location != INVALID_UNIFORM_LOCATION;
    }

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
        const UniformDesc*     Uniforms;
        usize                  UniformsCount = 0;
    };

    struct BufferDesc
    {
        usize       Size;
        const void* Data;
    };

    // A single indexed draw, fully described by backend resources. The backend
    // never learns what a mesh is: it is handed the buffers and the count.
    struct DrawCall
    {
        HPipeline Pipeline;
        HBuffer   Vertices;
        HBuffer   Indices;
        uint32    IndexCount = 0;
    };
}
