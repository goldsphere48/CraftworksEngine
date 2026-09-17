#include "null_graphics.h"

#include "graphics/graphics_adapter.h"
#include "utils/hash.h"
#include "utils/macros.h"

// A backend that allocates and accounts for resources but issues no commands.
// It exists to keep the adapter contract honest -- anything the OpenGL backend
// can only do through GL global state cannot be expressed here -- and to let
// tests drive the device layer without a window or a GL context.

namespace cw::graphics
{
    struct NullUniform
    {
        uint64 Hash = 0;
    };

    struct NullPipeline
    {
        NullUniform* Uniforms      = nullptr;
        usize        UniformsCount = 0;
    };

    struct NullBuffer
    {
        usize Size = 0;
    };

    static bool Initialize(void* window)
    {
        CW_UNUSED(window);

        return true;
    }

    static void Destroy()
    {
    }

    static void BeginFrame()
    {
    }

    static void EndFrame()
    {
    }

    static void UpdateViewport(int width, int height)
    {
        CW_UNUSED(width);
        CW_UNUSED(height);
    }

    static HPipeline CreatePipeline(const PipelineDesc* desc)
    {
        NullPipeline* pipeline  = new NullPipeline;
        pipeline->UniformsCount = desc->UniformsCount;
        pipeline->Uniforms      = new NullUniform[desc->UniformsCount]{};

        for (usize i = 0; i < desc->UniformsCount; ++i)
        {
            pipeline->Uniforms[i].Hash = utils::HashString(desc->Uniforms[i].Name);
        }

        return HPipeline{pipeline};
    }

    static void DestroyPipeline(const HPipeline pipeline)
    {
        NullPipeline* nullPipeline = static_cast<NullPipeline*>(pipeline.Id);
        delete[] nullPipeline->Uniforms;
        delete nullPipeline;
    }

    static void BindPipeline(const HPipeline pipeline)
    {
        CW_UNUSED(pipeline);
    }

    static HUniformLocation GetUniformLocation(const HPipeline pipeline, uint64 nameHash)
    {
        NullPipeline* nullPipeline = static_cast<NullPipeline*>(pipeline.Id);

        for (usize i = 0; i < nullPipeline->UniformsCount; ++i)
        {
            if (nullPipeline->Uniforms[i].Hash == nameHash)
            {
                return static_cast<HUniformLocation>(i);
            }
        }

        return INVALID_UNIFORM_LOCATION;
    }

    static void
    SetConstantV4(const HPipeline pipeline, const float* data, usize count, HUniformLocation location)
    {
        CW_UNUSED(pipeline);
        CW_UNUSED(data);
        CW_UNUSED(count);
        CW_UNUSED(location);
    }

    static void
    SetConstantM4(const HPipeline pipeline, const float* data, usize count, HUniformLocation location)
    {
        CW_UNUSED(pipeline);
        CW_UNUSED(data);
        CW_UNUSED(count);
        CW_UNUSED(location);
    }

    static HBuffer CreateBuffer(const BufferDesc* desc)
    {
        NullBuffer* buffer = new NullBuffer;
        buffer->Size       = desc->Size;
        return HBuffer{buffer};
    }

    static void DeleteBuffer(const HBuffer buffer)
    {
        delete static_cast<NullBuffer*>(buffer.Id);
    }

    static void Draw(const DrawCall* draw)
    {
        CW_UNUSED(draw);
    }

    void GetNullAdapter(GraphicsAdapter* adapter)
    {
        adapter->Initialize     = Initialize;
        adapter->Destroy        = Destroy;
        adapter->BeginFrame     = BeginFrame;
        adapter->EndFrame       = EndFrame;
        adapter->UpdateViewport = UpdateViewport;
        adapter->CreatePipeline = CreatePipeline;
        adapter->DestroyPipeline = DestroyPipeline;
        adapter->BindPipeline   = BindPipeline;
        adapter->GetUniformLocation = GetUniformLocation;
        adapter->SetConstantV4      = SetConstantV4;
        adapter->SetConstantM4      = SetConstantM4;
        adapter->CreateBuffer   = CreateBuffer;
        adapter->DeleteBuffer   = DeleteBuffer;
        adapter->Draw           = Draw;
    }
}
