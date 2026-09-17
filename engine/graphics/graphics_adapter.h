#pragma once

#include "graphics_types.h"

// The adapter is the whole contract a backend has to satisfy. Nothing above the
// device layer includes this header: engine code talks to graphics.h, which
// forwards to whichever adapter was bound at startup.

namespace cw::graphics
{
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

    typedef void (*FDraw)(const DrawCall* draw);

    typedef HUniformLocation (*FGetUniformLocation)(const HPipeline pipeline, uint64 nameHash);

    // Constants are written in whole vec4 / mat4 elements, count at a time. Anything
    // narrower is promoted by the caller and unpacked by the backend, which is the
    // only place that still knows the declared type.
    typedef void (*FSetConstantV4)(
        const HPipeline  pipeline,
        const float*     data,
        usize            count,
        HUniformLocation location
    );

    typedef void (*FSetConstantM4)(
        const HPipeline  pipeline,
        const float*     data,
        usize            count,
        HUniformLocation location
    );

    struct GraphicsAdapter
    {
        FInitialize         Initialize;
        FDestroy            Destroy;
        FBeginFrame         BeginFrame;
        FEndFrame           EndFrame;
        FUpdateViewport     UpdateViewport;
        FCreatePipeline     CreatePipeline;
        FDestroyPipeline    DestroyPipeline;
        FBindPipeline       BindPipeline;
        FGetUniformLocation GetUniformLocation;
        FSetConstantV4      SetConstantV4;
        FSetConstantM4      SetConstantM4;
        FCreateBuffer       CreateBuffer;
        FDeleteBuffer       DeleteBuffer;
        FDraw               Draw;

        ADAPTER_FAMILY Family = ADAPTER_FAMILY_NONE;
    };
}
