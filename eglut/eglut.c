/*
 * Copyright (C) 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <png.h>

#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "eglutint.h"

static struct eglut_state _eglut_state = {
        .api_mask = EGLUT_OPENGL_ES1_BIT,
        .window_width = 300,
        .window_height = 300,
        .window_fullscreen = EGLUT_WINDOWED,
        .verbose = 0,
        .num_windows = 0,
};

struct eglut_state *_eglut = &_eglut_state;

void
_eglutFatal(char *format, ...)
{
    va_list args;

    va_start(args, format);

    fprintf(stderr, "EGLUT: ");
    vfprintf(stderr, format, args);
    va_end(args);
    putc('\n', stderr);

    exit(1);
}

/* return current time (in milliseconds) */
int
_eglutNow(void)
{
    struct timeval tv;
#ifdef __VMS
    (void) gettimeofday(&tv, NULL );
#else
    struct timezone tz;
    (void) gettimeofday(&tv, &tz);
#endif
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void
_eglutDestroyWindow(struct eglut_window *win)
{
    if (_eglut->surface_type != EGL_PBUFFER_BIT)
        eglDestroySurface(_eglut->dpy, win->surface);

    _eglutNativeFiniWindow(win);

    eglDestroyContext(_eglut->dpy, win->context);
}

static EGLConfig
_eglutChooseConfig(void)
{
    EGLConfig config;
    EGLint config_attribs[32];
    EGLint renderable_type, num_configs, i;

    i = 0;
    config_attribs[i++] = EGL_RED_SIZE;
    config_attribs[i++] = 1;
    config_attribs[i++] = EGL_GREEN_SIZE;
    config_attribs[i++] = 1;
    config_attribs[i++] = EGL_BLUE_SIZE;
    config_attribs[i++] = 1;
    config_attribs[i++] = EGL_DEPTH_SIZE;
    config_attribs[i++] = 1;
    config_attribs[i++] = EGL_STENCIL_SIZE;
    config_attribs[i++] = 8;

    config_attribs[i++] = EGL_SURFACE_TYPE;
    config_attribs[i++] = _eglut->surface_type;

    config_attribs[i++] = EGL_RENDERABLE_TYPE;
    renderable_type = 0x0;
    if (_eglut->api_mask & EGLUT_OPENGL_BIT)
        renderable_type |= EGL_OPENGL_BIT;
    if (_eglut->api_mask & EGLUT_OPENGL_ES1_BIT)
        renderable_type |= EGL_OPENGL_ES_BIT;
    if (_eglut->api_mask & EGLUT_OPENGL_ES2_BIT)
        renderable_type |= EGL_OPENGL_ES2_BIT;
    if (_eglut->api_mask & EGLUT_OPENVG_BIT)
        renderable_type |= EGL_OPENVG_BIT;
    config_attribs[i++] = renderable_type;

    config_attribs[i] = EGL_NONE;

    if (!eglChooseConfig(_eglut->dpy,
                         config_attribs, &config, 1, &num_configs) || !num_configs)
        _eglutFatal("failed to choose a config");

    return config;
}

static struct eglut_window *
_eglutCreateWindow(const char *title, int x, int y, int w, int h, const char *icon)
{
    struct eglut_window *win;
    EGLint context_attribs[4];
    EGLint api, i;

    win = calloc(1, sizeof(*win));
    if (!win)
        _eglutFatal("failed to allocate window");

    win->config = _eglutChooseConfig();

    i = 0;
    context_attribs[i] = EGL_NONE;

    /* multiple APIs? */

    api = EGL_OPENGL_ES_API;
    if (_eglut->api_mask & EGLUT_OPENGL_BIT) {
        api = EGL_OPENGL_API;
    }
    else if (_eglut->api_mask & EGLUT_OPENVG_BIT) {
        api = EGL_OPENVG_API;
    }
    else if (_eglut->api_mask & EGLUT_OPENGL_ES2_BIT) {
        context_attribs[i++] = EGL_CONTEXT_CLIENT_VERSION;
        context_attribs[i++] = 2;
    }

    context_attribs[i] = EGL_NONE;

    eglBindAPI(api);
    win->context = eglCreateContext(_eglut->dpy,
                                    win->config, EGL_NO_CONTEXT, context_attribs);
    if (!win->context)
        _eglutFatal("failed to create context");

    _eglutNativeInitWindow(win, title, x, y, w, h, icon);
    switch (_eglut->surface_type) {
        case EGL_WINDOW_BIT:
            win->surface = eglCreateWindowSurface(_eglut->dpy,
                                                  win->config, win->native.u.window, NULL);
            break;
        case EGL_PIXMAP_BIT:
            win->surface = eglCreatePixmapSurface(_eglut->dpy,
                                                  win->config, win->native.u.pixmap, NULL);
            break;
        case EGL_PBUFFER_BIT:
            win->surface = win->native.u.surface;
            break;
        default:
            break;
    }
    if (win->surface == EGL_NO_SURFACE)
        _eglutFatal("failed to create surface");

    return win;
}

void
eglutInitAPIMask(int mask)
{
    _eglut->api_mask = mask;
}

void
eglutInitWindowSize(int width, int height)
{
    _eglut->window_width = width;
    _eglut->window_height = height;
}

void
eglutInit(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-display") == 0)
            _eglut->display_name = argv[++i];
        else if (strcmp(argv[i], "-info") == 0) {
            _eglut->verbose = 1;
        }
    }

    _eglutNativeInitDisplay();
    _eglut->dpy = eglGetDisplay(_eglut->native_dpy);

    if (!eglInitialize(_eglut->dpy, &_eglut->major, &_eglut->minor))
        _eglutFatal("failed to initialize EGL display");

    _eglut->init_time = _eglutNow();

    printf("EGL_VERSION = %s\n", eglQueryString(_eglut->dpy, EGL_VERSION));
    if (_eglut->verbose) {
        printf("EGL_VENDOR = %s\n", eglQueryString(_eglut->dpy, EGL_VENDOR));
        printf("EGL_EXTENSIONS = %s\n",
               eglQueryString(_eglut->dpy, EGL_EXTENSIONS));
        printf("EGL_CLIENT_APIS = %s\n",
               eglQueryString(_eglut->dpy, EGL_CLIENT_APIS));
    }
}

int
eglutGet(int state)
{
    int val;

    switch (state) {
        case EGLUT_ELAPSED_TIME:
            val = _eglutNow() - _eglut->init_time;
            break;
        case EGLUT_FULLSCREEN_MODE:
            val = _eglut->window_fullscreen;
            break;
        default:
            val = -1;
            break;
    }

    return val;
}

void
eglutIdleFunc(EGLUTidleCB func)
{
    _eglut->idle_cb = func;
}

void
eglutPostRedisplay(void)
{
    _eglut->redisplay = 1;
}

void
eglutMainLoop(void)
{
    struct eglut_window *win = _eglut->current;

    if (!win)
        _eglutFatal("no window is created\n");

    if (win->reshape_cb)
        win->reshape_cb(win->native.width, win->native.height);

    _eglutNativeEventLoop();
}

void
eglutFini(void)
{
    eglTerminate(_eglut->dpy);
    _eglutNativeFiniDisplay();
}

void
eglutDestroyWindow(int win)
{
    struct eglut_window *window = _eglut->current;

    if (window->index != win)
        return;

    eglMakeCurrent(_eglut->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    _eglutDestroyWindow(_eglut->current);
}

static void
_eglutDefaultKeyboard(unsigned char key)
{
    if (key == 27) {
        if (_eglut->current)
            eglutDestroyWindow(_eglut->current->index);
        eglutFini();

        exit(0);
    }
}

void*
_eglutReadPNG(char *filename, unsigned int *width, unsigned int *height) {
    FILE *file;
    char sig[8];
    int depth, colorType;
    png_size_t row_bytes;
    png_byte **rows;
    char* data;

    file = fopen(filename, "r");
    if (!file) {
        printf("_eglutReadPNG: fopen failed\n");
        return NULL;
    }
    png_struct *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        printf("_eglutReadPNG: png_create_read_struct error\n");
        return NULL;
    }
    png_info *info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        printf("_eglutReadPNG: png_create_info_struct error\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return NULL;
    }

    png_init_io(png_ptr, file);
    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, width, height, &depth, &colorType, NULL, NULL, NULL);

    if (colorType == PNG_COLOR_TYPE_RGB) {
        png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
        png_read_update_info(png_ptr, info_ptr);
    } else if (colorType != PNG_COLOR_TYPE_RGBA && colorType != PNG_COLOR_TYPE_PALETTE) {
        printf("_eglutReadPNG: unsupported color type\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }
    if (colorType == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png_ptr);
        else
            png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
        png_read_update_info(png_ptr, info_ptr);
    } else if (depth < 8 || png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_expand(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
    }
    if (depth == 16) {
        png_set_strip_16(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
    }

    row_bytes = png_get_rowbytes(png_ptr, info_ptr);

    rows = malloc(sizeof(png_byte *) * *height);
    data = malloc(row_bytes * *height);
    if (rows == NULL || data == NULL) {
        printf("_eglutReadPNG: malloc failed\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        if (rows != NULL)
            free(rows);
        return NULL;
    }
    for (unsigned int i = 0; i < *height; i++)
        rows[i] = &data[i * row_bytes];

    png_read_image(png_ptr, rows);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    free(rows);

    return data;
}

int
eglutCreateWindow(const char *title, const char *icon)
{
    struct eglut_window *win;

    win = _eglutCreateWindow(title, -1, -1, _eglut->window_width, _eglut->window_height, icon);

    win->index = _eglut->num_windows++;
    win->reshape_cb = NULL;
    win->display_cb = NULL;
    win->keyboard_cb = _eglutDefaultKeyboard;
    win->special_cb = NULL;
    win->mouse_cb = NULL;
    win->mouse_button_cb = NULL;

    if (!eglMakeCurrent(_eglut->dpy, win->surface, win->surface, win->context))
        _eglutFatal("failed to make window current");
    _eglut->current = win;

    return win->index;
}

int
eglutGetWindowX(void)
{
    struct eglut_window *win = _eglut->current;
    return win->native.x;
}

int
eglutGetWindowY(void)
{
    struct eglut_window *win = _eglut->current;
    return win->native.y;
}

int
eglutGetWindowWidth(void)
{
    struct eglut_window *win = _eglut->current;
    return win->native.width;
}

int
eglutGetWindowHeight(void)
{
    struct eglut_window *win = _eglut->current;
    return win->native.height;
}

void
eglutDisplayFunc(EGLUTdisplayCB func)
{
    struct eglut_window *win = _eglut->current;
    win->display_cb = func;

}

void
eglutReshapeFunc(EGLUTreshapeCB func)
{
    struct eglut_window *win = _eglut->current;
    win->reshape_cb = func;
}

void
eglutKeyboardFunc(EGLUTkeyboardCB func)
{
    struct eglut_window *win = _eglut->current;
    win->keyboard_cb = func;
}

void
eglutSpecialFunc(EGLUTspecialCB func)
{
    struct eglut_window *win = _eglut->current;
    win->special_cb = func;
}

void
eglutMouseFunc(EGLUTmouseCB func)
{
    struct eglut_window *win = _eglut->current;
    win->mouse_cb = func;
}

void
eglutMouseButtonFunc(EGLUTmouseButtonCB func)
{
    struct eglut_window *win = _eglut->current;
    win->mouse_button_cb = func;
}

void
eglutCloseWindowFunc(EGLUTcloseCB func)
{
    struct eglut_window *win = _eglut->current;
    win->close_cb = func;
}