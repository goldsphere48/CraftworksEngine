#ifdef CW_PLATFORM_WINDOWS

#include "debug/debug.h"
#include "opengl_renderer.h"

#include <windows.h>

#include <gl/gl.h>

#include "gl/glext.h"
#include "gl/wglext.h"

PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB    = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

#define LOAD_GL_PFN(func, pfn)                                  \
    do                                                          \
    {                                                           \
        func = reinterpret_cast<pfn>(wglGetProcAddress(#func)); \
        if (!func)                                              \
        {                                                       \
            CW_ERROR("Failed to load %s function", #func);      \
            return nullptr;                                     \
        }                                                       \
    } while (0)

namespace cw::graphics
{
    struct GLPlatformContext
    {
        HGLRC Context;
        HWND  Window;
        HDC   DeviceContext;
    };

    static constexpr const char* TEMP_WINDOW_CLASS = "CW_GL_TEMP_WINDOW_CLASS";

    GLPlatformContext* GLInitializePlatform(void* window)
    {
        HWND      hwnd     = static_cast<HWND>(window);
        HINSTANCE instance = GetModuleHandleA(nullptr);

        WNDCLASSEXA tempClass   = {};
        tempClass.cbSize        = sizeof(WNDCLASSEXA);
        tempClass.style         = CS_OWNDC;
        tempClass.lpfnWndProc   = DefWindowProcA;
        tempClass.hInstance     = instance;
        tempClass.lpszClassName = TEMP_WINDOW_CLASS;

        if (RegisterClassExA(&tempClass) == 0)
        {
            CW_ERROR("Failed to register temporary window class");
            return nullptr;
        }

        HWND dummy = CreateWindowExA(
            0,
            TEMP_WINDOW_CLASS,
            "CW GL Loader",
            WS_OVERLAPPED,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            nullptr,
            nullptr,
            instance,
            nullptr
        );

        CW_ASSERT(dummy);

        HDC fakeDC = GetDC(dummy);

        PIXELFORMATDESCRIPTOR fakeDesc;
        ZeroMemory(&fakeDesc, sizeof(PIXELFORMATDESCRIPTOR));
        fakeDesc.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
        fakeDesc.nVersion   = 1;
        fakeDesc.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        fakeDesc.iPixelType = PFD_TYPE_RGBA;
        fakeDesc.cColorBits = 32;
        fakeDesc.cAlphaBits = 8;
        fakeDesc.cDepthBits = 24;

        int fakePFDID = ChoosePixelFormat(fakeDC, &fakeDesc);
        if (fakePFDID == 0)
        {
            CW_ERROR("Failed to choose pixel format");
            return nullptr;
        }

        if (!SetPixelFormat(fakeDC, fakePFDID, &fakeDesc))
        {
            CW_ERROR("Failed to set pixel format");
            return nullptr;
        }

        HGLRC fakeRC = wglCreateContext(fakeDC);
        if (fakeRC == 0)
        {
            CW_ERROR("Failed to create wgl context");
            return nullptr;
        }

        if (!wglMakeCurrent(fakeDC, fakeRC))
        {
            CW_ERROR("Failed to make current context");
            return nullptr;
        }

        LOAD_GL_PFN(wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC);
        LOAD_GL_PFN(wglCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC);

        const int pixelAttribs[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     32,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            WGL_STENCIL_BITS_ARB,   8,
            WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
            WGL_SAMPLES_ARB,        4,
            0
        };

        HDC dc = GetDC(hwnd);

        int  pixelFormatId = 0;
        UINT numFormats    = 0;

        if (!wglChoosePixelFormatARB(dc, pixelAttribs, nullptr, 1, &pixelFormatId, &numFormats) || numFormats == 0)
        {
            CW_ERROR("Failed to choose pixel format");
            return nullptr;
        }

        PIXELFORMATDESCRIPTOR desc;
        DescribePixelFormat(dc, pixelFormatId, sizeof(desc), &desc);
        SetPixelFormat(dc, pixelFormatId, &desc);

        const int major = 4;
        const int minor = 5;

        int contextAttribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, major,
            WGL_CONTEXT_MINOR_VERSION_ARB, minor,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
        if (rc == nullptr)
        {
            CW_ERROR("Failed to create opengl context");
            return nullptr;
        }

        if (!wglMakeCurrent(dc, rc))
        {
            CW_ERROR("Failed to make opengl context current");
            return nullptr;
        }

        wglDeleteContext(fakeRC);
        ReleaseDC(dummy, fakeDC);
        DestroyWindow(dummy);
        UnregisterClassA(TEMP_WINDOW_CLASS, instance);

        GLPlatformContext* ctx = new GLPlatformContext();
        ctx->Window            = hwnd;
        ctx->Context           = rc;
        ctx->DeviceContext     = dc;

        return ctx;
    }

    void GLDestroyPlatform(GLPlatformContext* ctx)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(ctx->Context);
        delete ctx;
    }

    void GLSwapBuffers(GLPlatformContext* ctx)
    {
        SwapBuffers(ctx->DeviceContext);
    }
}
#endif
