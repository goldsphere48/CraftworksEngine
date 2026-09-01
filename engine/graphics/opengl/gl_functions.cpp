#include "gl_functions.h"

PFNGLCREATESHADERPROC             glCreateShader             = nullptr;
PFNGLDELETESHADERPROC             glDeleteShader             = nullptr;
PFNGLSHADERSOURCEPROC             glShaderSource             = nullptr;
PFNGLCOMPILESHADERPROC            glCompileShader            = nullptr;
PFNGLGETSHADERIVPROC              glGetShaderiv              = nullptr;
PFNGLGETSHADERINFOLOGPROC         glGetShaderInfoLog         = nullptr;
PFNGLCREATEPROGRAMPROC            glCreateProgram            = nullptr;
PFNGLDELETEPROGRAMPROC            glDeleteProgram            = nullptr;
PFNGLATTACHSHADERPROC             glAttachShader             = nullptr;
PFNGLDETACHSHADERPROC             glDetachShader             = nullptr;
PFNGLLINKPROGRAMPROC              glLinkProgram              = nullptr;
PFNGLGETPROGRAMIVPROC             glGetProgramiv             = nullptr;
PFNGLGETPROGRAMINFOLOGPROC        glGetProgramInfoLog        = nullptr;
PFNGLPROGRAMPARAMETERIPROC        glProgramParameteri        = nullptr;
PFNGLGETPROGRAMBINARYPROC         glGetProgramBinary         = nullptr;
PFNGLPROGRAMBINARYPROC            glProgramBinary            = nullptr;
PFNGLUSEPROGRAMPROC               glUseProgram               = nullptr;
PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation       = nullptr;
PFNGLGETATTRIBLOCATIONPROC        glGetAttribLocation        = nullptr;
PFNGLUNIFORM1FPROC                glUniform1f                = nullptr;
PFNGLUNIFORM2FPROC                glUniform2f                = nullptr;
PFNGLUNIFORM3FPROC                glUniform3f                = nullptr;
PFNGLUNIFORM4FPROC                glUniform4f                = nullptr;
PFNGLUNIFORM1FVPROC               glUniform1fv               = nullptr;
PFNGLUNIFORM2FVPROC               glUniform2fv               = nullptr;
PFNGLUNIFORM3FVPROC               glUniform3fv               = nullptr;
PFNGLUNIFORM4FVPROC               glUniform4fv               = nullptr;
PFNGLUNIFORMMATRIX4FVPROC         glUniformMatrix4fv         = nullptr;
PFNGLGENBUFFERSPROC               glGenBuffers               = nullptr;
PFNGLDELETEBUFFERSPROC            glDeleteBuffers            = nullptr;
PFNGLBINDBUFFERPROC               glBindBuffer               = nullptr;
PFNGLBUFFERDATAPROC               glBufferData               = nullptr;
PFNGLGENVERTEXARRAYSPROC          glGenVertexArrays          = nullptr;
PFNGLDELETEVERTEXARRAYSPROC       glDeleteVertexArrays       = nullptr;
PFNGLBINDVERTEXARRAYPROC          glBindVertexArray          = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer      = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray  = nullptr;
PFNGLCREATEBUFFERSPROC            glCreateBuffers            = nullptr;
PFNGLCREATEVERTEXARRAYSPROC       glCreateVertexArrays       = nullptr;
PFNGLVERTEXARRAYATTRIBFORMATPROC  glVertexArrayAttribFormat  = nullptr;
PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding = nullptr;
PFNGLENABLEVERTEXARRAYATTRIBPROC  glEnableVertexArrayAttrib  = nullptr;
PFNGLVERTEXARRAYVERTEXBUFFERPROC  glVertexArrayVertexBuffer  = nullptr;
PFNGLVERTEXARRAYELEMENTBUFFERPROC glVertexArrayElementBuffer = nullptr;
PFNGLNAMEDBUFFERDATAPROC          glNamedBufferData          = nullptr;
PFNGLNAMEDBUFFERSTORAGEPROC       glNamedBufferStorage       = nullptr;
PFNGLNAMEDBUFFERSUBDATAPROC       glNamedBufferSubData       = nullptr;
PFNGLDEBUGMESSAGECALLBACKPROC     glDebugMessageCallback     = nullptr;
PFNGLDEBUGMESSAGECONTROLPROC      glDebugMessageControl      = nullptr;

namespace cw::graphics
{
    bool LoadGLCore()
    {
        CW_LOAD_GL(glCreateShader, PFNGLCREATESHADERPROC);
        CW_LOAD_GL(glDeleteShader, PFNGLDELETESHADERPROC);
        CW_LOAD_GL(glShaderSource, PFNGLSHADERSOURCEPROC);
        CW_LOAD_GL(glCompileShader, PFNGLCOMPILESHADERPROC);
        CW_LOAD_GL(glGetShaderiv, PFNGLGETSHADERIVPROC);
        CW_LOAD_GL(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC);
        CW_LOAD_GL(glCreateProgram, PFNGLCREATEPROGRAMPROC);
        CW_LOAD_GL(glDeleteProgram, PFNGLDELETEPROGRAMPROC);
        CW_LOAD_GL(glAttachShader, PFNGLATTACHSHADERPROC);
        CW_LOAD_GL(glDetachShader, PFNGLDETACHSHADERPROC);
        CW_LOAD_GL(glLinkProgram, PFNGLLINKPROGRAMPROC);
        CW_LOAD_GL(glGetProgramiv, PFNGLGETPROGRAMIVPROC);
        CW_LOAD_GL(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC);
        CW_LOAD_GL(glProgramParameteri, PFNGLPROGRAMPARAMETERIPROC);
        CW_LOAD_GL(glGetProgramBinary, PFNGLGETPROGRAMBINARYPROC);
        CW_LOAD_GL(glProgramBinary, PFNGLPROGRAMBINARYPROC);
        CW_LOAD_GL(glUseProgram, PFNGLUSEPROGRAMPROC);
        CW_LOAD_GL(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC);
        CW_LOAD_GL(glGetAttribLocation, PFNGLGETATTRIBLOCATIONPROC);
        CW_LOAD_GL(glUniform1f, PFNGLUNIFORM1FPROC);
        CW_LOAD_GL(glUniform2f, PFNGLUNIFORM2FPROC);
        CW_LOAD_GL(glUniform3f, PFNGLUNIFORM3FPROC);
        CW_LOAD_GL(glUniform4f, PFNGLUNIFORM4FPROC);
        CW_LOAD_GL(glUniform1fv, PFNGLUNIFORM1FVPROC);
        CW_LOAD_GL(glUniform2fv, PFNGLUNIFORM2FVPROC);
        CW_LOAD_GL(glUniform3fv, PFNGLUNIFORM3FVPROC);
        CW_LOAD_GL(glUniform4fv, PFNGLUNIFORM4FVPROC);
        CW_LOAD_GL(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC);
        CW_LOAD_GL(glGenBuffers, PFNGLGENBUFFERSPROC);
        CW_LOAD_GL(glDeleteBuffers, PFNGLDELETEBUFFERSPROC);
        CW_LOAD_GL(glBindBuffer, PFNGLBINDBUFFERPROC);
        CW_LOAD_GL(glBufferData, PFNGLBUFFERDATAPROC);
        CW_LOAD_GL(glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC);
        CW_LOAD_GL(glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC);
        CW_LOAD_GL(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC);
        CW_LOAD_GL(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC);
        CW_LOAD_GL(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC);
        CW_LOAD_GL(glCreateVertexArrays, PFNGLCREATEVERTEXARRAYSPROC);
        CW_LOAD_GL(glVertexArrayAttribFormat, PFNGLVERTEXARRAYATTRIBFORMATPROC);
        CW_LOAD_GL(glVertexArrayAttribBinding, PFNGLVERTEXARRAYATTRIBBINDINGPROC);
        CW_LOAD_GL(glEnableVertexArrayAttrib, PFNGLENABLEVERTEXARRAYATTRIBPROC);
        CW_LOAD_GL(glVertexArrayVertexBuffer, PFNGLVERTEXARRAYVERTEXBUFFERPROC);
        CW_LOAD_GL(glDebugMessageCallback, PFNGLDEBUGMESSAGECALLBACKPROC);
        CW_LOAD_GL(glDebugMessageControl, PFNGLDEBUGMESSAGECONTROLPROC);
        CW_LOAD_GL(glCreateBuffers, PFNGLCREATEBUFFERSPROC);
        CW_LOAD_GL(glVertexArrayElementBuffer, PFNGLVERTEXARRAYELEMENTBUFFERPROC);
        CW_LOAD_GL(glNamedBufferData, PFNGLNAMEDBUFFERDATAPROC);
        CW_LOAD_GL(glNamedBufferStorage, PFNGLNAMEDBUFFERSTORAGEPROC);
        CW_LOAD_GL(glNamedBufferSubData, PFNGLNAMEDBUFFERSUBDATAPROC);

        return true;
    }
}
