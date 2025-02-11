#include <EGL/egl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

typedef struct ESContext {
    EGLint width;
    EGLint height;
    char fb_multi_buffer[12];
    char device[20];
    char title[50];
    EGLContext eglContext;
    EGLDisplay display;
    EGLSurface surface;
    EGLNativeDisplayType native_display;
    EGLNativeWindowType native_window;
    EGLNativePixmapType native_pixmap;
} ESContext;

EGLBoolean ESContextInit(ESContext* context);
const char* eglGetErrorStr(EGLint eglError);
EGLBoolean WinCreate(ESContext* context);

int main(int argc, char* argv[]) {
    EGLint attribList[] = {
        EGL_RED_SIZE,       5,
        EGL_GREEN_SIZE,     6,
        EGL_BLUE_SIZE,      5,
        EGL_ALPHA_SIZE,     EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     EGL_DONT_CARE,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_NONE
    };

    EGLint major = 0;
    EGLint minor = 0;
    EGLint error = 0;
    EGLint numConfigs = 0;
    EGLConfig config;
    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    ESContext esContext;

    ESContextInit(&esContext);

    int option_arg_index = 1;
    while (option_arg_index < argc) {
        if (strcmp(argv[option_arg_index], "--width") == 0) {
            esContext.width = atoi(argv[++option_arg_index]);
        }
        else if (strcmp(argv[option_arg_index], "--height") == 0) {
            esContext.height = atoi(argv[++option_arg_index]);
        }
        else if (strcmp(argv[option_arg_index], "--device") == 0) {
            option_arg_index++;
            snprintf(esContext.device, strlen(argv[option_arg_index]) + 1, "%s", argv[option_arg_index]);
        }
        else if (strcmp(argv[option_arg_index], "--fb-multi-buffer") == 0) {
            option_arg_index++;
            snprintf(esContext.fb_multi_buffer, strlen(argv[option_arg_index]) + 1, "%s", argv[option_arg_index]);
        }

        option_arg_index++;
    }

    printf("Application parameters: \n\tWidth=%d\n\tHeight=%d\n\tDevice=%s\n\tFB_MULTI_BUFFER=%s\n",
        esContext.width, esContext.height, esContext.device, esContext.fb_multi_buffer);

    if (!WinCreate(&esContext)) {
        fprintf(stderr, "WinCreate failed\n");

        return -1;
    }

    printf("native_display: %s, native_window: %lu\n", (esContext.native_display ? "NOT_NULL" : "NULL"), esContext.native_window);

    esContext.display = eglGetDisplay(esContext.native_display);
    if (esContext.display == EGL_NO_DISPLAY) {
        fprintf(stderr, "eglGetDisplay(native_display) failed, EGL_NO_DISPLAY\n");

        esContext.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (esContext.display == EGL_NO_DISPLAY) {
            fprintf(stderr, "eglGetDisplay(EGL_DEFAULT_DISPLAY) failed, EGL_NO_DISPLAY\n");

            return -1;
        }
    }

    if (!eglInitialize(esContext.display, &major, &minor)) {
        error = eglGetError();
        fprintf(stderr, "eglInitialize failed - %s\n", eglGetErrorStr(error));

        return -1;
    }

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        error = eglGetError();
        fprintf(stderr, "eglBindAPI failed - %s\n", eglGetErrorStr(error));

        return -1;
    }

    // Get configs
    if (!eglGetConfigs(esContext.display, NULL, 0, &numConfigs)) {
        error = eglGetError();
        fprintf(stderr, "eglGetConfigs failed - %s\n", eglGetErrorStr(error));

        return -1;
    }

    // Choose config
    if (!eglChooseConfig(esContext.display, attribList, &config, 1, &numConfigs)) {
        error = eglGetError();
        fprintf(stderr, "eglChooseConfig failed - %s\n", eglGetErrorStr(error));

        return -1;
    }

    // Create a native window surface
    esContext.surface = eglCreateWindowSurface(esContext.display, config, esContext.native_window, NULL);
    if (esContext.surface == EGL_NO_SURFACE) {
        error = eglGetError();
        fprintf(stderr, "eglCreateWindowSurface failed, EGL_NO_SURFACE - %s\n", eglGetErrorStr(error));

        // Create a native pixmap surface
        esContext.surface = eglCreatePixmapSurface(esContext.display, config, esContext.native_pixmap, NULL);
        if (esContext.surface == EGL_NO_SURFACE) {
            error = eglGetError();
            fprintf(stderr, "eglCreatePixmapSurface failed, EGL_NO_SURFACE - %s\n", eglGetErrorStr(error));

            return -1;
        }
    }

    // Create a GL context
    esContext.eglContext = eglCreateContext(esContext.display, config, EGL_NO_CONTEXT, contextAttribs);
    if (esContext.eglContext == EGL_NO_CONTEXT) {
        error = eglGetError();
        fprintf(stderr, "eglCreateContext failed, EGL_NO_CONTEXT - %s\n", eglGetErrorStr(error));

        return -1;
    }

    // Make the context current
    if (!eglMakeCurrent(esContext.display, esContext.surface, esContext.surface, esContext.eglContext)) {
        error = eglGetError();
        fprintf(stderr, "eglMakeCurrent failed - %s\n", eglGetErrorStr(error));

        return -1;
    }

    while (true) {
        static int i = 0;
        i++;
    }

    return 0;
}

EGLBoolean ESContextInit(ESContext* context) {
    if (!context) {
        fprintf(stderr, "eglMakeCurrent failed");

        return EGL_FALSE;
    }

    const char default_fb_multi_buffer[] = "";
    const char default_device[] = "/dev/dri/card0";

    memset(context, 0, sizeof(ESContext));

    context->width = 1280;
    context->height = 480;
    snprintf(context->fb_multi_buffer, strlen(default_fb_multi_buffer) + 1, "%s", default_fb_multi_buffer);
    snprintf(context->device, strlen(default_device) + 1, "%s", default_device);

    return EGL_TRUE;
}

const char* eglGetErrorStr(EGLint eglError) {
    switch(eglError) {
        case EGL_SUCCESS:
            return "The last function succeeded without error.";
            break;
        case EGL_NOT_INITIALIZED:
            return "EGL is not initialized, or could not be initialized, for the specified EGL display connection.";
            break;
        case EGL_BAD_ACCESS:
            return "EGL cannot access a requested resource (for example a context is bound in another thread).";
            break;
        case EGL_BAD_ALLOC:
            return "EGL failed to allocate resources for the requested operation.";
            break;
        case EGL_BAD_ATTRIBUTE:
            return "An unrecognized attribute or attribute value was passed in the attribute list.";
            break;
        case EGL_BAD_CONTEXT:
            return "An EGLContext argument does not name a valid EGL rendering context.";
            break;
        case EGL_BAD_CONFIG:
            return "An EGLConfig argument does not name a valid EGL frame buffer configuration.";
            break;
        case EGL_BAD_CURRENT_SURFACE:
            return "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid.";
            break;
        case EGL_BAD_DISPLAY:
            return "An EGLDisplay argument does not name a valid EGL display connection.";
            break;
        case EGL_BAD_SURFACE:
            return "An EGLSurface argument does not name a valid surface (window, pixel buffer or pixmap) configured for GL rendering.";
            break;
        case EGL_BAD_MATCH:
            return "Arguments are inconsistent (for example, a valid context requires buffers not supplied by a valid surface).";
            break;
        case EGL_BAD_PARAMETER:
            return "One or more argument values are invalid.";
            break;
        case EGL_BAD_NATIVE_PIXMAP:
            return "A NativePixmapType argument does not refer to a valid native pixmap.";
            break;
        case EGL_BAD_NATIVE_WINDOW:
            return "A NativeWindowType argument does not refer to a valid native window.";
            break;
        case EGL_CONTEXT_LOST:
            return "A power management event has occurred. The application must destroy all contexts and reinitialise OpenGL ES state and objects to continue rendering.";
            break;
    }

    return "";
}

EGLBoolean WinCreate(ESContext* context) {
    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    Window x_window;
    Display* x_display;

    // X11 native display initialization
    x_display = XOpenDisplay(NULL);
    if (x_display == NULL) {
        return EGL_FALSE;
    }

    root = DefaultRootWindow(x_display);

    swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask | StructureNotifyMask;
    x_window = XCreateWindow(
        x_display, root,
        0, 0, context->width, context->height, 0,
        CopyFromParent, InputOutput,
        CopyFromParent, CWEventMask,
        &swa
    );

    xattr.override_redirect = 0;
    XChangeWindowAttributes(x_display, x_window, CWOverrideRedirect, &xattr);

    hints.input = 1;
    hints.flags = InputHint;
    XSetWMHints(x_display, x_window, &hints);

    // make the window visible on the screen
    XMapWindow(x_display, x_window);
    XStoreName(x_display, x_window, context->title);

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom(x_display, "_NET_WM_STATE", 0);

    memset(&xev, 0, sizeof(xev));
    xev.type                 = ClientMessage;
    xev.xclient.window       = x_window;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = 0;

    XSendEvent(
        x_display,
        DefaultRootWindow (x_display),
        0,
        SubstructureNotifyMask,
        &xev
    );

    context->native_display = (EGLNativeDisplayType)x_display;
    context->native_window = (EGLNativeWindowType)x_window;

    return EGL_TRUE;
}
