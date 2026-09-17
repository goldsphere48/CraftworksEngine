#include "opengl_graphics.h"

#include "gl_functions.h"
#include "graphics/graphics_adapter.h"
#include "utils/hash.h"
#include "utils/macros.h"

#include <stdio.h>
#include <stdlib.h>

namespace cw::graphics
{
    struct GLUniform
    {
        uint64       Hash;
        GLint        Location;
        UNIFORM_TYPE Type;
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

    static bool CompileGLSL(const char* source, SHADER_TYPE type, GLuint* shaderOut)
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

        *shaderOut = id;

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

    static GLuint CreateProgramFromSource(const char* vertexSource, const char* fragmentSource)
    {
        GLuint vertex   = 0;
        GLuint fragment = 0;

        bool success =
            CompileGLSL(vertexSource, SHADER_TYPE_VERTEX, &vertex) &&
            CompileGLSL(fragmentSource, SHADER_TYPE_FRAGMENT, &fragment);

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
            return {};
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
        
        for (usize i = 0; i < desc->UniformsCount; ++i)
        {
            const UniformDesc* uDesc = &desc->Uniforms[i];
            GLUniform*         u     = &glPipeline->Uniforms[i];

            u->Location = glGetUniformLocation(program, uDesc->Name);
            u->Hash     = utils::HashString(uDesc->Name);
            u->Type     = uDesc->Type;
        }

        glPipeline->Program = program;
        glPipeline->Stride  = offset;
        glPipeline->VAO     = vao;

        return HPipeline{glPipeline};
    }

    // The location handed out is an index into the pipeline's own uniform table,
    // not a GL location: the GL location and the declared type stay in here, which
    // is what lets the adapter expose only vec4 and mat4 writes.
    static HUniformLocation GetUniformLocation(const HPipeline pipeline, uint64 nameHash)
    {
        GLPipeline* glPipeline = static_cast<GLPipeline*>(pipeline.Id);
        for (usize i = 0; i < glPipeline->UniformsCount; ++i)
        {
            if (glPipeline->Uniforms[i].Hash != nameHash)
            {
                continue;
            }

            // Declared but never read: the linker stripped it, there is nowhere to write.
            if (glPipeline->Uniforms[i].Location < 0)
            {
                return INVALID_UNIFORM_LOCATION;
            }

            return static_cast<HUniformLocation>(i);
        }

        return INVALID_UNIFORM_LOCATION;
    }

    static const GLUniform* ResolveUniform(const HPipeline pipeline, HUniformLocation location)
    {
        if (!IsValidLocation(location))
        {
            return nullptr;
        }

        GLPipeline* glPipeline = static_cast<GLPipeline*>(pipeline.Id);
        if (static_cast<usize>(location) >= glPipeline->UniformsCount)
        {
            return nullptr;
        }

        return &glPipeline->Uniforms[location];
    }

    static void
    SetConstantV4(const HPipeline pipeline, const float* data, usize count, HUniformLocation location)
    {
        const GLUniform* uniform = ResolveUniform(pipeline, location);
        if (uniform == nullptr)
        {
            return;
        }

        const GLsizei elements = static_cast<GLsizei>(count);

        switch (uniform->Type)
        {
            case UNIFORM_TYPE_FLOAT:
                glUniform1fv(uniform->Location, elements, data);
                break;
            case UNIFORM_TYPE_VEC2:
                glUniform2fv(uniform->Location, elements, data);
                break;
            case UNIFORM_TYPE_VEC3:
                glUniform3fv(uniform->Location, elements, data);
                break;
            case UNIFORM_TYPE_VEC4:
                glUniform4fv(uniform->Location, elements, data);
                break;
            case UNIFORM_TYPE_MAT4:
                CW_ERROR("SetConstantV4 used on a mat4 uniform");
                break;
        }
    }

    static void
    SetConstantM4(const HPipeline pipeline, const float* data, usize count, HUniformLocation location)
    {
        const GLUniform* uniform = ResolveUniform(pipeline, location);
        if (uniform == nullptr)
        {
            return;
        }

        if (uniform->Type != UNIFORM_TYPE_MAT4)
        {
            CW_ERROR("SetConstantM4 used on a non-mat4 uniform");
            return;
        }

        glUniformMatrix4fv(uniform->Location, static_cast<GLsizei>(count), GL_FALSE, data);
    }

    static void DestroyPipeline(const HPipeline pipeline)
    {
        GLPipeline* glPipeline = (GLPipeline*)pipeline.Id;
        glDeleteProgram(glPipeline->Program);
        glDeleteVertexArrays(1, &glPipeline->VAO);
        delete[] glPipeline->Uniforms;
        delete glPipeline;
    }

    static void BindPipeline(const HPipeline pipeline)
    {
        GLPipeline* glPipeline = (GLPipeline*)pipeline.Id;
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
        buffer->Id       = vbo;
        return HBuffer{buffer};
    }

    static void DeleteBuffer(const HBuffer buffer)
    {
        GLBuffer* glBuffer = (GLBuffer*)buffer.Id;
        glDeleteBuffers(1, &glBuffer->Id);
        delete glBuffer;
    }

    static void Draw(const DrawCall* draw)
    {
        GLPipeline* glPipeline = (GLPipeline*)draw->Pipeline.Id;

        GLuint stride = glPipeline->Stride;
        GLuint vao    = glPipeline->VAO;
        GLuint vbo    = ((GLBuffer*)draw->Vertices.Id)->Id;
        GLuint ibo    = ((GLBuffer*)draw->Indices.Id)->Id;

        glVertexArrayVertexBuffer(vao, 0, vbo, 0, stride);
        glVertexArrayElementBuffer(vao, ibo);
        glDrawElements(GL_TRIANGLES, draw->IndexCount, GL_UNSIGNED_INT, nullptr);
    }

    void GetGLAdapter(GraphicsAdapter* adapter)
    {
        adapter->Initialize         = Initialize;
        adapter->Destroy            = Destroy;
        adapter->BeginFrame         = BeginFrame;
        adapter->EndFrame           = EndFrame;
        adapter->CreatePipeline     = CreatePipeline;
        adapter->DestroyPipeline    = DestroyPipeline;
        adapter->BindPipeline       = BindPipeline;
        adapter->CreateBuffer       = CreateBuffer;
        adapter->GetUniformLocation = GetUniformLocation;
        adapter->SetConstantV4      = SetConstantV4;
        adapter->SetConstantM4      = SetConstantM4;
        adapter->DeleteBuffer       = DeleteBuffer;
        adapter->Draw               = Draw;
        adapter->UpdateViewport     = UpdateViewport;
    }
}
