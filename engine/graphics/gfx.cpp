#include "gfx.h"

#include "math/vector.h"

#include <cstring>

namespace cw::gfx
{
    using namespace cw::graphics;

    static void BindMaterial(GraphicsContext* ctx, const Material* material)
    {
        const PipelineResource* pipeline = material->Pipeline;

        BindPipeline(ctx, pipeline->BackendPipeline);

        for (usize i = 0; i < pipeline->UniformsCount; ++i)
        {
            const PipelineUniform&   uniform   = pipeline->Uniforms[i];
            const MaterialParameter& parameter = material->Parameters[i];

            switch (uniform.Type)
            {
                case UNIFORM_TYPE_FLOAT:
                    SetUniformFloat(ctx, uniform.BackendUniform, parameter.Value[0]);
                    break;
                case UNIFORM_TYPE_VEC2:
                    SetUniformVec2(
                        ctx,
                        uniform.BackendUniform,
                        math::vec::Make<float>(parameter.Value[0], parameter.Value[1])
                    );
                    break;
                case UNIFORM_TYPE_VEC3:
                    SetUniformVec3(
                        ctx,
                        uniform.BackendUniform,
                        math::vec::Make<float>(
                            parameter.Value[0],
                            parameter.Value[1],
                            parameter.Value[2]
                        )
                    );
                    break;
                case UNIFORM_TYPE_VEC4:
                    SetUniformVec4(
                        ctx,
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
                {
                    Mat4 value;
                    memcpy(value.Data, parameter.Value, sizeof(value.Data));
                    SetUniformMat4(ctx, uniform.BackendUniform, &value);
                    break;
                }
            }
        }
    }

    void DrawMesh(GraphicsContext* ctx, const Material* material, const Mesh* mesh)
    {
        BindMaterial(ctx, material);

        graphics::DrawMesh(ctx, mesh, material->Pipeline->BackendPipeline);
    }
}
