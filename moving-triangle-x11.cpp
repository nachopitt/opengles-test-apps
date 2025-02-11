#include <EGL/egl.h>
#include <stdio.h>

const char* eglGetErrorStr(EGLint eglError);

int main(int argc, char* argv[]) {
    EGLint attribList[] = {
        EGL_RED_SIZE,       5,
        EGL_GREEN_SIZE,     6,
        EGL_BLUE_SIZE,      5,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     8,
        EGL_STENCIL_SIZE,   8,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_NONE
    };

    EGLDisplay display;
    EGLint major = 0;
    EGLint minor = 0;
    EGLint error = 0;
    EGLint numConfigs = 0;
    EGLConfig config;
    EGLNativeWindowType window;
    EGLNativePixmapType pixmap;
    EGLSurface surface;
    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    EGLContext context;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        fprintf(stderr, "eglGetDisplay failed, EGL_NO_DISPLAY\n");

        return -1;
    }

    if (!eglInitialize(display, &major, &minor)) {
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
    if (!eglGetConfigs(display, NULL, 0, &numConfigs)) {
        error = eglGetError();
        fprintf(stderr, "eglGetConfigs failed - %s\n", eglGetErrorStr(error));

        return -1;
    }

    // Choose config
    if (!eglChooseConfig(display, attribList, &config, 1, &numConfigs)) {
        error = eglGetError();
        fprintf(stderr, "eglChooseConfig failed - %s\n", eglGetErrorStr(error));

        return -1;
    }

    // Create a window surface
    surface = eglCreateWindowSurface(display, config, window, NULL);
    if (surface == EGL_NO_SURFACE) {
        error = eglGetError();
        fprintf(stderr, "eglCreateWindowSurface failed, EGL_NO_SURFACE - %s\n", eglGetErrorStr(error));

        // Create a pixmap surface
        surface = eglCreatePixmapSurface(display, config, pixmap, NULL);
        if (surface == EGL_NO_SURFACE) {
            error = eglGetError();
            fprintf(stderr, "eglCreatePixmapSurface failed, EGL_NO_SURFACE - %s\n", eglGetErrorStr(error));

            return -1;
        }
    }

    // Create a GL context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        error = eglGetError();
        fprintf(stderr, "eglCreateContext failed, EGL_NO_CONTEXT - %s\n", eglGetErrorStr(error));

        return -1;
    }

    // Make the context current
    if (!eglMakeCurrent(display, surface, surface, context)) {
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