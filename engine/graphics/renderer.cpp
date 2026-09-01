#include "renderer.h"

#include "assets/assets_manager.h"
#include "logger/log.h"
#include "math/vector.h"
#include "opengl/opengl_renderer.h"
#include "pipeline_manager.h"

#include <cstring>

namespace cw::graphics
{
    static RenderBackend g_Backend;
    
    GraphicsContext* Create(const GraphicsParams* params)
    {
        g_Backend.BackendType = params->Backend;

        switch (g_Backend.BackendType)
        {
            case RENDER_BACKEND_OPENGL:
                GetGLBindings(&g_Backend);
                break;
            default:
                return nullptr;
        }

        if (!g_Backend.Initialize(params->Window))
        {
            return nullptr;
        }

        GraphicsContext* ctx = new GraphicsContext;
        ctx->PipelineManager = CreatePipelineManager();
        ctx->MaterialContext = material::CreateContext(ctx->PipelineManager);

        OnResize(ctx, params->Viewport.X, params->Viewport.Y);

        return ctx;
    }

    void OnResize(GraphicsContext* ctx, int width, int height)
    {
        ctx->Viewport.Size.X = width;
        ctx->Viewport.Size.Y = height;

        g_Backend.UpdateViewport(width, height);
    }

    HPipeline CreatePipeline(const PipelineDesc* desc)
    {
        return g_Backend.CreatePipeline(desc);
    }

    void DestroyPipeline(HPipeline pipeline)
    {
        g_Backend.DestroyPipeline(pipeline);
    }

    void GetUniform(const HPipeline pipeline, uint64 nameHash, HUniform* outUniform)
    {
        g_Backend.GetUniform(pipeline, nameHash, outUniform);
    }

    void SetFloat(const HUniform uniform, float value)
    {
        g_Backend.SetFloat(uniform, value);
    }

    void SetVec2(const HUniform uniform, Vec2 value)
    {
        g_Backend.SetVec2(uniform, value);
    }

    void SetVec3(const HUniform uniform, Vec3 value)
    {
        g_Backend.SetVec3(uniform, value);
    }

    void SetVec4(const HUniform uniform, Vec4 value)
    {
        g_Backend.SetVec4(uniform, value);
    }

    void SetMat4(const HUniform uniform, const float* value)
    {
        g_Backend.SetMat4(uniform, value);
    }

    void BeginFrame()
    {
        g_Backend.BeginFrame();
    }

    void EndFrame()
    {
        g_Backend.EndFrame();
    }

    void DrawMesh(const material::Material* material, const Mesh* mesh)
    {
        g_Backend.BindPipeline(material->Pipeline->BackendPipeline);
        for (usize i = 0; i < material->Pipeline->UniformsCount; ++i)
        {
            const PipelineUniform&             uniform   = material->Pipeline->Uniforms[i];
            const material::MaterialParameter& parameter = material->Parameters[i];
            switch (uniform.Type)
            {
                case UNIFORM_TYPE_FLOAT:
                    SetFloat(uniform.BackendUniform, parameter.Value[0]);
                    break;
                case UNIFORM_TYPE_VEC2:
                    SetVec2(
                        uniform.BackendUniform,
                        math::vec::Make<float>(parameter.Value[0], parameter.Value[1])
                    );
                    break;
                case UNIFORM_TYPE_VEC3:
                    SetVec3(
                        uniform.BackendUniform,
                        math::vec::Make<float>(
                            parameter.Value[0],
                            parameter.Value[1],
                            parameter.Value[2]
                        )
                    );
                    break;
                case UNIFORM_TYPE_VEC4:
                    SetVec4(
                        uniform.BackendUniform,
                        math::vec::Make<float>(
                            parameter.Value[0],
                            parameter.Value[1],
                            parameter.Value[2],
                            parameter.Value[3]
                        )
                    );
                    break;
                case UNIFORM_TYPE_MAT4:
                    SetMat4(uniform.BackendUniform, &parameter.Value[0]);
                    break;
            }
        }
        g_Backend.DrawMesh(mesh, material->Pipeline->BackendPipeline);
    }

    Mesh* CreateMesh(const void* vertices, usize vertices_size, const uint32* indices, usize index_count)
    {
        BufferDesc vertexDesc = {
            .Count = 0,
            .Size  = vertices_size,
            .Data  = (void*)vertices,
        };

        BufferDesc indexDesc = {
            .Count = index_count,
            .Size  = index_count * sizeof(uint32),
            .Data  = (void*)indices,
        };

        Mesh* mesh     = new Mesh;
        mesh->Vertices = g_Backend.CreateBuffer(&vertexDesc);
        mesh->Indicies = g_Backend.CreateBuffer(&indexDesc);
        return mesh;
    }

    void DestroyMesh(const Mesh* mesh)
    {
        g_Backend.DeleteBuffer(mesh->Vertices);
        g_Backend.DeleteBuffer(mesh->Indicies);
        delete mesh;
    }

    void Destroy(GraphicsContext* ctx)
    {
        material::DestroyContext(ctx->MaterialContext);
        DestroyPipelineManager(ctx->PipelineManager);
        g_Backend.Destroy();

        delete ctx;
    }
}
