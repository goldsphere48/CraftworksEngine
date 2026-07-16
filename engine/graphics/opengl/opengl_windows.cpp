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
            return false;                                       \
        }                                                       \
    } while (0)

namespace cw::graphics
{
    static const char* CLASS_NAME = "CW_GL_TEMP_WINDOW_CLASS";

    struct GLWindowsContext
    {
        HGLRC     Context;
        HWND      Window;
        HDC       DeviceContext;
    };

    static GLWindowsContext* g_Context = nullptr;

    class DummyContext
    {
    public:
        HWND      Window = nullptr;
        HDC       DeviceContext = nullptr;
        HGLRC     Context = nullptr;
        HINSTANCE Instance;
        
        ~DummyContext()
        {
            if (Context)
            {
                wglDeleteContext(Context);
            }
            if (DeviceContext)
            {
                ReleaseDC(Window, DeviceContext);
            }
            if (Window)
            {
                DestroyWindow(Window);
                UnregisterClassA(CLASS_NAME, Instance);
            }
        }
    };

    static bool LoadGLExtensions()
    {
        LOAD_GL_PFN(wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC);
        LOAD_GL_PFN(wglCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC);

        return true;
    }

    bool GLInitializePlatform(void* window)
    {
        CW_ASSERT(!g_Context);
        
        HWND      hwnd     = static_cast<HWND>(window);
        HINSTANCE instance = GetModuleHandleA(nullptr);

        DummyContext dummyCtx = {};
        dummyCtx.Instance = instance;

        WNDCLASSEXA tempClass   = {};
        tempClass.cbSize        = sizeof(WNDCLASSEXA);
        tempClass.style         = CS_OWNDC;
        tempClass.lpfnWndProc   = DefWindowProcA;
        tempClass.hInstance     = instance;
        tempClass.lpszClassName = CLASS_NAME;

        if (RegisterClassExA(&tempClass) == 0)
        {
            CW_ERROR("Failed to register temporary window class");
            return false;
        }

        dummyCtx.Window = CreateWindowExA(
            0,
            CLASS_NAME,
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

        if (!dummyCtx.Window)
        {
            CW_ERROR("Failed to create dummy window");
            return false;
        }

        dummyCtx.DeviceContext = GetDC(dummyCtx.Window);

        PIXELFORMATDESCRIPTOR fakeDesc;
        ZeroMemory(&fakeDesc, sizeof(PIXELFORMATDESCRIPTOR));
        fakeDesc.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
        fakeDesc.nVersion   = 1;
        fakeDesc.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        fakeDesc.iPixelType = PFD_TYPE_RGBA;
        fakeDesc.cColorBits = 32;
        fakeDesc.cAlphaBits = 8;
        fakeDesc.cDepthBits = 24;

        int fakePFDID = ChoosePixelFormat(dummyCtx.DeviceContext, &fakeDesc);
        if (fakePFDID == 0)
        {
            CW_ERROR("Failed to choose pixel format");
            return false;
        }

        if (!SetPixelFormat(dummyCtx.DeviceContext, fakePFDID, &fakeDesc))
        {
            CW_ERROR("Failed to set pixel format");
            return false;
        }

        dummyCtx.Context = wglCreateContext(dummyCtx.DeviceContext);
        if (dummyCtx.Context == 0)
        {
            CW_ERROR("Failed to create wgl context");
            return false;
        }

        if (!wglMakeCurrent(dummyCtx.DeviceContext, dummyCtx.Context))
        {
            CW_ERROR("Failed to make current context");
            return false;
        }

        LoadGLExtensions();
        
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

        if (!dc)
        {
            CW_ERROR("Failed to get dc");
            return false;
        }

        int  pixelFormatId = 0;
        UINT numFormats    = 0;

        if (!wglChoosePixelFormatARB(dc, pixelAttribs, nullptr, 1, &pixelFormatId, &numFormats) || numFormats == 0)
        {
            CW_ERROR("Failed to choose pixel format");
            return false;
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
            return false;
        }

        if (!wglMakeCurrent(dc, rc))
        {
            wglDeleteContext(rc);
            CW_ERROR("Failed to make opengl context current");
            return false;
        }

        g_Context                = new GLWindowsContext;
        g_Context->Window        = hwnd;
        g_Context->Context       = rc;
        g_Context->DeviceContext = dc;

        return true;
    }

    void GLDestroyPlatform()
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(g_Context->Context);
        ReleaseDC(g_Context->Window, g_Context->DeviceContext);
        delete g_Context;
    }

    void GLSwapBuffers()
    {
        SwapBuffers(g_Context->DeviceContext);
    }
}
#endif
