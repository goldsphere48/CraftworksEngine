#include "opengl_renderer.h"

#include "gl_functions.h"
#include "graphics/renderer_backend.h"
#include "utils/macros.h"

#include <stdio.h>
#include <stdlib.h>

namespace cw::graphics
{
    struct GLPipeline
    {
        GLuint Program;
        GLuint VAO;
        GLuint Stride;
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

    static GLuint CreateProgramFromSource(const ShaderDesc* desc)
    {
        GLuint vertex   = 0;
        GLuint fragment = 0;

        bool success = CompileGLSL(desc->VertexSource, SHADER_TYPE_VERTEX, &vertex)
                       && CompileGLSL(desc->FragmentSource, SHADER_TYPE_FRAGMENT, &fragment);

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

    static GLuint CreateProgramFromBinary(const void* data, size_t size)
    {
        if (data == nullptr || size < sizeof(GLenum))
        {
            return 0;
        }

        GLenum      format     = *(const GLenum*)data;
        const void* binary     = (const char*)data + sizeof(GLenum);
        GLsizei     binarySize = (GLsizei)(size - sizeof(GLenum));

        GLuint program = glCreateProgram();
        glProgramBinary(program, format, binary, binarySize);

        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (linked == GL_FALSE)
        {
            glDeleteProgram(program);
            return 0;
        }

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
        GLuint program = desc->Binary != nullptr
                             ? CreateProgramFromBinary(desc->Binary, desc->BinarySize)
                             : CreateProgramFromSource(&desc->Source);

        if (program == 0)
        {
            return nullptr;
        }

        GLuint vao;
        glCreateVertexArrays(1, &vao);

        int offset = 0;
        for (int i = 0; i < desc->Layout.AttributeCount; i++)
        {
            const VertexAttribute* attrib   = &desc->Layout.Attributes[i];
            const GLVertexAttrib   glAttrib = ConvertToGLVertexAttribute(attrib->Format);

            glVertexArrayAttribFormat(
                vao,
                attrib->Location,
                glAttrib.Components,
                glAttrib.Type,
                glAttrib.Normalized,
                offset
            );

            glVertexArrayAttribBinding(vao, attrib->Location, 0);
            glEnableVertexArrayAttrib(vao, attrib->Location);

            offset += glAttrib.Size;
        }

        GLPipeline* glPipeline = new GLPipeline;
        glPipeline->Program    = program;
        glPipeline->Stride     = offset;
        glPipeline->VAO        = vao;

        return glPipeline;
    }

    static void GetPipelineBinary(HPipeline pipeline, void** out_binary, size_t* out_size)
    {
        GLPipeline* glPipeline = (GLPipeline*)pipeline;

        GLint length = 0;
        glGetProgramiv(glPipeline->Program, GL_PROGRAM_BINARY_LENGTH, &length);
        if (length == 0)
        {
            *out_binary = nullptr;
            *out_size   = 0;
            return;
        }

        size_t total  = sizeof(GLenum) + length;
        char*  buffer = (char*)(malloc(total));

        GLenum  format  = 0;
        GLsizei written = 0;

        glGetProgramBinary(
            glPipeline->Program,
            length,
            &written,
            &format,
            buffer + sizeof(GLenum)
        );

        *reinterpret_cast<GLenum*>(buffer) = format;

        *out_binary = buffer;
        *out_size   = total;
    }

    static const char* GetPipelineCacheId()
    {
        static char id[512];
        if (id[0] != 0)
        {
            return id;
        }

        const char* vendor   = (const char*)glGetString(GL_VENDOR);
        const char* renderer = (const char*)glGetString(GL_RENDERER);
        const char* version  = (const char*)glGetString(GL_VERSION);

        snprintf(id, sizeof(id), "PIPELINE|%s|%s|%s", vendor, renderer, version);

        return id;
    }

    static void DestroyPipeline(const HPipeline pipeline)
    {
        GLPipeline* glPipeline = (GLPipeline*)pipeline;
        glDeleteProgram(glPipeline->Program);
        glDeleteVertexArrays(1, &glPipeline->VAO);
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
        backend->GetPipelineBinary  = GetPipelineBinary;
        backend->GetPipelineCacheId = GetPipelineCacheId;
        backend->CreateBuffer       = CreateBuffer;
        backend->DeleteBuffer       = DeleteBuffer;
        backend->DrawMesh           = DrawMesh;
    }
}
