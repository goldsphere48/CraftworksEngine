#include "opengl_renderer.h"

#include "gl_functions.h"
#include "graphics/renderer_backend.h"
#include "utils/hash.h"
#include "utils/macros.h"

#include <stdio.h>
#include <stdlib.h>

namespace cw::graphics
{
    struct GLUniform
    {
        uint64 Hash;
        int    Location;
    };
    
    struct GLPipeline
    {
        GLuint     Program;
        GLuint     VAO;
        GLuint     Stride;
        GLUniform* Uniforms;
        usize      UniformsCount;
    };

    struct GLVertexAttrib
    {
        GLint     Size;
        GLenum    Type;
        GLboolean Normalized;
        GLint     Components;
    };

    struct GLBuffer
    {
        GLuint Id;
        GLuint Count;
        GLuint Size;
    };

#ifdef CW_BUILD_DEBUG
    static void APIENTRY GLDebugCallback(
        GLenum        source,
        GLenum        type,
        GLuint        id,
        GLenum        severity,
        GLsizei       length,
        const GLchar* message,
        const void*   userParam
    )
    {
        CW_UNUSED(source);
        CW_UNUSED(type);
        CW_UNUSED(id);
        CW_UNUSED(length);
        CW_UNUSED(userParam);

        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:
                CW_ERROR("[GL] %s", message);
                break;
            case GL_DEBUG_SEVERITY_MEDIUM:
            case GL_DEBUG_SEVERITY_LOW:
                CW_WARNING("[GL] %s", message);
                break;
            default:
                CW_DEBUG("[GL] %s", message);
                break;
        }
    }

    static void EnableGLDebugOutput()
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, nullptr);
        glDebugMessageControl(
            GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DEBUG_SEVERITY_NOTIFICATION,
            0,
            nullptr,
            GL_FALSE
        );
    }
#endif

    static void UpdateViewport(int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    static bool CompileGLSL(const char* source, SHADER_TYPE type, GLuint* shader_out)
    {
        GLenum shaderType = type == SHADER_TYPE_VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;

        GLuint id = glCreateShader(shaderType);
        glShaderSource(id, 1, &source, nullptr);
        glCompileShader(id);

        GLint success = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            GLint length = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

            char* logBuffer = (char*)malloc(length);
            glGetShaderInfoLog(id, length, &length, logBuffer);

            glDeleteShader(id);

            CW_ERROR("Shader compile failed: %s", logBuffer);

            free(logBuffer);

            return false;
        }

        *shader_out = id;

        return true;
    }

    static bool Initialize(void* window)
    {
        bool result = GLInitializePlatform(window);
        if (!result)
        {
            return false;
        }

#ifdef CW_BUILD_DEBUG
        EnableGLDebugOutput();
#endif

        return true;
    }

    static void Destroy()
    {
        GLDestroyPlatform();
    }

    static void BeginFrame()
    {
        glClearColor(0.3f, 0.3f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    static void EndFrame()
    {
        GLSwapBuffers();
    }

    static GLuint CreateProgramFromSource(const char* vertex_source, const char* fragment_source)
    {
        GLuint vertex   = 0;
        GLuint fragment = 0;

        bool success =
            CompileGLSL(vertex_source, SHADER_TYPE_VERTEX, &vertex) &&
            CompileGLSL(fragment_source, SHADER_TYPE_FRAGMENT, &fragment);

        if (!success)
        {
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);

        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            char* infoLog = (char*)malloc(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(program);

            free(infoLog);

            return 0;
        }

        glDetachShader(program, vertex);
        glDetachShader(program, fragment);
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        return program;
    }

    static GLVertexAttrib ConvertToGLVertexAttribute(VERTEX_FORMAT format)
    {
        switch (format)
        {
            case VERTEX_FORMAT_FLOAT:
                return {4, GL_FLOAT, false, 1};
            case VERTEX_FORMAT_FLOAT2:
                return {8, GL_FLOAT, false, 2};
            case VERTEX_FORMAT_FLOAT3:
                return {12, GL_FLOAT, false, 3};
            case VERTEX_FORMAT_FLOAT4:
                return {16, GL_FLOAT, false, 4};
            case VERTEX_FORMAT_UBYTE4:
                return {4, GL_UNSIGNED_BYTE, false, 4};
        }

        return {};
    }

    static HPipeline CreatePipeline(const PipelineDesc* desc)
    {
        GLuint program = CreateProgramFromSource(desc->VertexSource, desc->FragmentSource);
        if (program == 0)
        {
            return nullptr;
        }

        GLuint vao;
        glCreateVertexArrays(1, &vao);

        int offset = 0;
        for (int i = 0; i < desc->AttributeCount; i++)
        {
            const VertexAttribute* attrib = &desc->Attributes[i];
            const GLint           location = glGetAttribLocation(program, attrib->Name);
            const GLVertexAttrib  glAttrib = ConvertToGLVertexAttribute(attrib->Format);

            if (location >= 0)
            {
                glVertexArrayAttribFormat(
                    vao,
                    location,
                    glAttrib.Components,
                    glAttrib.Type,
                    glAttrib.Normalized,
                    offset
                );

                glVertexArrayAttribBinding(vao, location, 0);
                glEnableVertexArrayAttrib(vao, location);
            }

            offset += glAttrib.Size;
        }

        GLPipeline* glPipeline = new GLPipeline;
        glPipeline->Uniforms = new GLUniform[desc->UniformsCount];
        glPipeline->UniformsCount = desc->UniformsCount;
        
        for (int i = 0; i < desc->UniformsCount; ++i)
        {
            UniformDesc* u_desc = &desc->Uniforms[i];
            GLUniform* u = &glPipeline->Uniforms[i];
            GLint location = glGetUniformLocation(program, u_desc->Name);
            u->Location = location;
            u->Hash = utils::HashString(u_desc->Name);
        }

        glPipeline->Program = program;
        glPipeline->Stride  = offset;
        glPipeline->VAO     = vao;

        return glPipeline;
    }

    static void GetUniform(const HPipeline pipeline, uint64 nameHash, HUniform* outUniform)
    {
        GLPipeline* glPipeline = static_cast<GLPipeline*>(pipeline);
        for (int i = 0; i < glPipeline->UniformsCount; ++i)
        {
            if (glPipeline->Uniforms[i].Hash == nameHash)
            {
                *outUniform = static_cast<HUniform>(&glPipeline->Uniforms[i]);
                return;
            }
        }

        *outUniform = nullptr;
    }

    static void SetFloat(HUniform uniform, float value)
    {
        GLUniform* u = static_cast<GLUniform*>(uniform);
        glUniform1fv(u->Location, 1, &value);
    }

    static void SetVec2(HUniform uniform, Vec2 value)
    {
        GLUniform* u = static_cast<GLUniform*>(uniform);
        glUniform2fv(u->Location, 1, value.Data);
    }


    static void SetVec3(HUniform uniform, Vec3 value)
    {
        GLUniform* u = static_cast<GLUniform*>(uniform);
        glUniform3fv(u->Location, 1, value.Data);
    }


    static void SetVec4(HUniform uniform, Vec4 value)
    {
        GLUniform* u = static_cast<GLUniform*>(uniform);
        glUniform4fv(u->Location, 1, value.Data);
    }


    static void SetMat4(HUniform uniform, const float* value)
    {
        GLUniform* u = static_cast<GLUniform*>(uniform);
        glUniformMatrix4fv(u->Location, 1, false, value);
    }

    static void DestroyPipeline(const HPipeline pipeline)
    {
        GLPipeline* glPipeline = (GLPipeline*)pipeline;
        glDeleteProgram(glPipeline->Program);
        glDeleteVertexArrays(1, &glPipeline->VAO);
        delete[] glPipeline->Uniforms;
        delete glPipeline;
    }

    static void BindPipeline(const HPipeline pipeline)
    {
        GLPipeline* glPipeline = (GLPipeline*)pipeline;
        glUseProgram(glPipeline->Program);
        glBindVertexArray(glPipeline->VAO);
    }

    static HBuffer CreateBuffer(const BufferDesc* desc)
    {
        GLuint vbo;
        glCreateBuffers(1, &vbo);
        glNamedBufferStorage(vbo, desc->Size, desc->Data, 0);

        GLBuffer* buffer = new GLBuffer;
        buffer->Size     = (GLuint)desc->Size;
        buffer->Count    = (GLuint)desc->Count;
        buffer->Id       = vbo;
        return buffer;
    }

    static void DeleteBuffer(const HBuffer buffer)
    {
        GLBuffer* glBuffer = (GLBuffer*)buffer;
        glDeleteBuffers(1, &glBuffer->Id);
        delete glBuffer;
    }

    static void DrawMesh(const Mesh* mesh, HPipeline pipeline)
    {
        GLPipeline* glPipeline = (GLPipeline*)pipeline;

        GLuint    stride   = glPipeline->Stride;
        GLuint    vao      = glPipeline->VAO;
        GLBuffer* indicies = (GLBuffer*)mesh->Indicies;
        GLuint    vbo      = ((GLBuffer*)mesh->Vertices)->Id;
        GLuint    ibo      = indicies->Id;
        GLuint    program  = glPipeline->Program;

        glVertexArrayVertexBuffer(vao, 0, vbo, 0, stride);
        glVertexArrayElementBuffer(vao, ibo);
        glDrawElements(GL_TRIANGLES, indicies->Count, GL_UNSIGNED_INT, nullptr);
    }

    void GetGLBindings(RenderBackend* backend)
    {
        backend->Initialize         = Initialize;
        backend->Destroy            = Destroy;
        backend->BeginFrame         = BeginFrame;
        backend->EndFrame           = EndFrame;
        backend->CreatePipeline     = CreatePipeline;
        backend->DestroyPipeline    = DestroyPipeline;
        backend->BindPipeline       = BindPipeline;
        backend->CreateBuffer       = CreateBuffer;
        backend->GetUniform         = GetUniform;
        backend->SetFloat           = SetFloat;
        backend->SetVec2            = SetVec2;
        backend->SetVec3            = SetVec3;
        backend->SetVec4            = SetVec4;
        backend->SetMat4            = SetMat4;
        backend->DeleteBuffer       = DeleteBuffer;
        backend->DrawMesh           = DrawMesh;
        backend->UpdateViewport     = UpdateViewport;
    }
}
