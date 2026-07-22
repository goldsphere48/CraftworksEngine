#pragma once

#include "logger/log.h"

#ifdef CW_PLATFORM_WINDOWS
    #include <windows.h>
#endif

#include <gl/gl.h>

#include "gl/glext.h"

extern PFNGLCREATESHADERPROC            glCreateShader;
extern PFNGLDELETESHADERPROC            glDeleteShader;
extern PFNGLSHADERSOURCEPROC            glShaderSource;
extern PFNGLCOMPILESHADERPROC           glCompileShader;
extern PFNGLGETSHADERIVPROC             glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC           glCreateProgram;
extern PFNGLDELETEPROGRAMPROC           glDeleteProgram;
extern PFNGLATTACHSHADERPROC            glAttachShader;
extern PFNGLDETACHSHADERPROC            glDetachShader;
extern PFNGLLINKPROGRAMPROC             glLinkProgram;
extern PFNGLGETPROGRAMIVPROC            glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
extern PFNGLPROGRAMPARAMETERIPROC       glProgramParameteri;
extern PFNGLGETPROGRAMBINARYPROC        glGetProgramBinary;
extern PFNGLPROGRAMBINARYPROC           glProgramBinary;
extern PFNGLUSEPROGRAMPROC              glUseProgram;
extern PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
extern PFNGLUNIFORM4FPROC               glUniform4f;
extern PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv;

extern PFNGLGENBUFFERSPROC              glGenBuffers;
extern PFNGLDELETEBUFFERSPROC           glDeleteBuffers;
extern PFNGLBINDBUFFERPROC              glBindBuffer;
extern PFNGLBUFFERDATAPROC              glBufferData;

extern PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
extern PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

extern PFNGLCREATEVERTEXARRAYSPROC      glCreateVertexArrays;
extern PFNGLVERTEXARRAYATTRIBFORMATPROC glVertexArrayAttribFormat;
extern PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding;
extern PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
extern PFNGLVERTEXARRAYVERTEXBUFFERPROC glVertexArrayVertexBuffer;

extern PFNGLDEBUGMESSAGECALLBACKPROC    glDebugMessageCallback;
extern PFNGLDEBUGMESSAGECONTROLPROC     glDebugMessageControl;

namespace cw::graphics
{
    void* GLGetProcAddress(const char* name);

    bool LoadGLCore();
}

#define CW_LOAD_GL(func, pfn)                                                \
    do                                                                       \
    {                                                                        \
        func = reinterpret_cast<pfn>(cw::graphics::GLGetProcAddress(#func)); \
        if (!func)                                                           \
        {                                                                    \
            CW_ERROR("Failed to load GL function %s", #func);                \
            return false;                                                    \
        }                                                                    \
    } while (0)
