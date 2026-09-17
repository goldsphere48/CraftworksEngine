#include "render.h"

#include "constant_buffer.h"
#include "logger/log.h"
#include "material.h"
#include "math/vector.h"
#include "render_private.h"

#include <algorithm>

namespace cw::render
{
    RenderContext* CreateRenderContext(const RenderParams* params)
    {
        if (params == nullptr || params->Graphics == nullptr || params->MaxRenderObjects == 0)
        {
            return nullptr;
        }

        RenderContext* ctx  = new RenderContext{};
        ctx->Graphics       = params->Graphics;
        ctx->ObjectCapacity = params->MaxRenderObjects;
        ctx->Objects        = new RenderObject[params->MaxRenderObjects]{};
        ctx->Entries        = new RenderEntry[params->MaxRenderObjects]{};

        return ctx;
    }

    void DestroyRenderContext(RenderContext* ctx)
    {
        if (ctx == nullptr)
        {
            return;
        }

        delete[] ctx->Entries;
        delete[] ctx->Objects;

        delete ctx;
    }

    graphics::GraphicsContext* GetGraphicsContext(RenderContext* ctx)
    {
        return ctx->Graphics;
    }

    void SetCamera(RenderContext* ctx, const Camera* camera)
    {
        ctx->Camera = *camera;
    }

    void BeginFrame(RenderContext* ctx)
    {
        // The buffer was taken once at startup: a frame never allocates.
        ctx->ObjectCount = 0;

        graphics::BeginFrame(ctx->Graphics);
    }

    void EndFrame(RenderContext* ctx)
    {
        graphics::EndFrame(ctx->Graphics);
    }

    // Objects sharing a pipeline and a mesh land next to each other, which is what
    // lets the draw loop below skip rebinding.
    static uint32 MakeBatchKey(const RenderObject* object)
    {
        const uint64 pipeline = reinterpret_cast<uint64>(object->Material->Pipeline);
        const uint64 mesh     = reinterpret_cast<uint64>(object->Mesh);
        const uint64 mixed    = (pipeline * 1099511628211ull) ^ (mesh * 14695981039346656037ull);

        return static_cast<uint32>(mixed >> 32) ^ static_cast<uint32>(mixed);
    }

    // 8 bits of pass, 32 bits of batch, 24 bits of explicit order.
    static uint64 MakeSortKey(const RenderObject* object, uint32 batchKey)
    {
        return (static_cast<uint64>(object->MajorOrder) << 56) |
               (static_cast<uint64>(batchKey) << 24) |
               (static_cast<uint64>(object->Order) & 0xFFFFFFull);
    }

    void AddRenderObject(RenderContext* ctx, const RenderObject* object)
    {
        if (object->Material == nullptr || object->Mesh == nullptr)
        {
            return;
        }

        if (ctx->ObjectCount == ctx->ObjectCapacity)
        {
            CW_ERROR("Render list is full (%zu objects), dropping object", ctx->ObjectCapacity);
            return;
        }

        const usize index = ctx->ObjectCount++;

        ctx->Objects[index] = *object;
        ctx->Entries[index] = {
            .SortKey = MakeSortKey(object, MakeBatchKey(object)),
            .Index   = static_cast<uint32>(index),
        };
    }

    static void ApplyCameraConstants(RenderContext* ctx, const Pipeline* pipeline)
    {
        ApplyBuiltinMat4(ctx, pipeline, BUILTIN_UNIFORM_VIEW, &ctx->Camera.View);
        ApplyBuiltinMat4(ctx, pipeline, BUILTIN_UNIFORM_PROJECTION, &ctx->Camera.Projection);
    }

    static void ApplyMaterialConstants(RenderContext* ctx, const Material* material)
    {
        const Pipeline* pipeline = material->Pipeline;

        for (usize i = 0; i < pipeline->UniformsCount; ++i)
        {
            ApplyUniform(ctx, pipeline, pipeline->Uniforms[i], material->Parameters[i].Elements);
        }
    }

    void DrawRenderList(RenderContext* ctx)
    {
        std::sort(
            ctx->Entries,
            ctx->Entries + ctx->ObjectCount,
            [](const RenderEntry& a, const RenderEntry& b) { return a.SortKey < b.SortKey; }
        );

        const Pipeline* boundPipeline = nullptr;
        const Material* boundMaterial = nullptr;

        for (usize i = 0; i < ctx->ObjectCount; ++i)
        {
            const RenderObject& object   = ctx->Objects[ctx->Entries[i].Index];
            const Pipeline*     pipeline = object.Material->Pipeline;

            if (pipeline != boundPipeline)
            {
                boundPipeline = pipeline;
                graphics::BindPipeline(ctx->Graphics, pipeline->BackendPipeline);
                ApplyCameraConstants(ctx, pipeline);
                boundMaterial = nullptr;
            }

            if (object.Material != boundMaterial)
            {
                boundMaterial = object.Material;
                ApplyMaterialConstants(ctx, boundMaterial);
            }

            ApplyBuiltinMat4(ctx, pipeline, BUILTIN_UNIFORM_MODEL, &object.WorldTransform);

            if (object.Constants != nullptr)
            {
                ApplyConstants(ctx, pipeline, object.Constants);
            }

            const graphics::DrawCall draw = {
                .Pipeline   = pipeline->BackendPipeline,
                .Vertices   = object.Mesh->Vertices,
                .Indices    = object.Mesh->Indices,
                .IndexCount = object.Mesh->IndexCount,
            };

            graphics::Draw(ctx->Graphics, &draw);
        }
    }
}
