#include "pipeline.h"

#include "logger/log.h"
#include "render_private.h"
#include "utils/hash.h"

#include <cstring>

namespace cw::render
{
    struct BuiltinUniformDesc
    {
        const char*            Name;
        graphics::UNIFORM_TYPE Type;
    };

    // Index-matched to BUILTIN_UNIFORMS.
    constexpr BuiltinUniformDesc g_BuiltinUniforms[BUILTIN_UNIFORM_COUNT] =
    {
        { "u_Model",      graphics::UNIFORM_TYPE_MAT4 },
        { "u_View",       graphics::UNIFORM_TYPE_MAT4 },
        { "u_Projection", graphics::UNIFORM_TYPE_MAT4 },
    };

    static char* CopyString(const char* value)
    {
        const usize length = strlen(value);
        char*       copy   = new char[length + 1];
        memcpy(copy, value, length + 1);
        return copy;
    }

    usize GetUniformValueCount(graphics::UNIFORM_TYPE type)
    {
        switch (type)
        {
            case graphics::UNIFORM_TYPE_FLOAT:
                return 1;
            case graphics::UNIFORM_TYPE_VEC2:
                return 2;
            case graphics::UNIFORM_TYPE_VEC3:
                return 3;
            case graphics::UNIFORM_TYPE_VEC4:
                return 4;
            case graphics::UNIFORM_TYPE_MAT4:
                return 16;
        }

        return 0;
    }

    bool FindPipelineUniform(const Pipeline* pipeline, const char* name, usize* outIndex)
    {
        const uint64 nameHash = utils::HashString(name);
        for (usize i = 0; i < pipeline->UniformsCount; ++i)
        {
            const PipelineUniform& uniform = pipeline->Uniforms[i];
            if (uniform.NameHash == nameHash && strcmp(uniform.Name, name) == 0)
            {
                *outIndex = i;
                return true;
            }
        }

        return false;
    }

    // Naming rules belong to the renderer, not to the file format: a uniform may
    // not shadow a builtin, and two uniforms may not collide by name or by hash.
    static bool ValidateUniformNames(const PipelineCreateDesc* desc)
    {
        for (usize i = 0; i < desc->UniformCount; ++i)
        {
            const char*  name     = desc->Uniforms[i].Name;
            const uint64 nameHash = utils::HashString(name);

            for (usize j = 0; j < BUILTIN_UNIFORM_COUNT; ++j)
            {
                if (strcmp(g_BuiltinUniforms[j].Name, name) == 0)
                {
                    CW_ERROR("%s: uniform \"%s\" is reserved by engine", desc->DebugName, name);

                    return false;
                }
            }

            for (usize j = 0; j < i; ++j)
            {
                const char* otherName = desc->Uniforms[j].Name;
                if (utils::HashString(otherName) != nameHash)
                {
                    continue;
                }

                if (strcmp(otherName, name) == 0)
                {
                    CW_ERROR(
                        "%s: uniform \"%s\" is declared more than once",
                        desc->DebugName,
                        name
                    );
                }
                else
                {
                    CW_ERROR(
                        "%s: uniform hash collision between \"%s\" and \"%s\"",
                        desc->DebugName,
                        otherName,
                        name
                    );
                }

                return false;
            }
        }

        return true;
    }

    static graphics::HPipeline
    CreateBackendPipeline(RenderContext* ctx, const PipelineCreateDesc* desc)
    {
        const usize            uniformsCount = desc->UniformCount + BUILTIN_UNIFORM_COUNT;
        graphics::UniformDesc* uniforms      = new graphics::UniformDesc[uniformsCount];

        for (usize i = 0; i < BUILTIN_UNIFORM_COUNT; ++i)
        {
            const BuiltinUniformDesc& builtin = g_BuiltinUniforms[i];
            uniforms[i] = {builtin.Type, builtin.Name};
        }

        for (usize i = 0; i < desc->UniformCount; ++i)
        {
            const PipelineUniformDesc& uniform = desc->Uniforms[i];
            uniforms[i + BUILTIN_UNIFORM_COUNT] = {uniform.Type, uniform.Name};
        }

        const graphics::PipelineDesc backendDesc = {
            .VertexSource   = desc->VertexSource,
            .FragmentSource = desc->FragmentSource,
            .Attributes     = desc->Attributes,
            .AttributeCount = static_cast<int>(desc->AttributeCount),
            .Uniforms       = uniforms,
            .UniformsCount  = uniformsCount,
        };

        graphics::HPipeline pipeline = graphics::CreatePipeline(ctx->Graphics, &backendDesc);

        delete[] uniforms;

        return pipeline;
    }

    Pipeline* CreatePipeline(RenderContext* ctx, const PipelineCreateDesc* desc)
    {
        if (ctx == nullptr || desc == nullptr)
        {
            return nullptr;
        }

        if (!ValidateUniformNames(desc))
        {
            return nullptr;
        }

        graphics::HPipeline backendPipeline = CreateBackendPipeline(ctx, desc);
        if (!graphics::IsValid(backendPipeline))
        {
            CW_ERROR("%s: failed to create pipeline", desc->DebugName);
            return nullptr;
        }

        Pipeline* pipeline        = new Pipeline{};
        pipeline->BackendPipeline = backendPipeline;
        pipeline->UniformsCount   = desc->UniformCount;
        pipeline->Uniforms        = new PipelineUniform[desc->UniformCount]{};

        // A builtin the shader never reads is stripped by the driver. That is not an
        // error: the engine simply has nowhere to write it.
        for (usize i = 0; i < BUILTIN_UNIFORM_COUNT; ++i)
        {
            pipeline->Builtins[i] = graphics::GetUniformLocation(
                ctx->Graphics,
                pipeline->BackendPipeline,
                utils::HashString(g_BuiltinUniforms[i].Name)
            );
        }

        for (usize i = 0; i < pipeline->UniformsCount; ++i)
        {
            const PipelineUniformDesc& source  = desc->Uniforms[i];
            PipelineUniform&           uniform = pipeline->Uniforms[i];

            uniform.Name     = CopyString(source.Name);
            uniform.NameHash = utils::HashString(source.Name);
            uniform.Type     = source.Type;

            uniform.Location = graphics::GetUniformLocation(
                ctx->Graphics,
                pipeline->BackendPipeline,
                uniform.NameHash
            );

            if (!graphics::IsValidLocation(uniform.Location))
            {
                CW_ERROR(
                    "%s: backend did not create uniform \"%s\"",
                    desc->DebugName,
                    uniform.Name
                );
                DestroyPipeline(ctx, pipeline);
                return nullptr;
            }
        }

        return pipeline;
    }

    void DestroyPipeline(RenderContext* ctx, Pipeline* pipeline)
    {
        if (pipeline == nullptr)
        {
            return;
        }

        if (graphics::IsValid(pipeline->BackendPipeline))
        {
            graphics::DestroyPipeline(ctx->Graphics, pipeline->BackendPipeline);
        }

        for (usize i = 0; i < pipeline->UniformsCount; ++i)
        {
            delete[] pipeline->Uniforms[i].Name;
        }

        delete[] pipeline->Uniforms;
        delete pipeline;
    }
}
