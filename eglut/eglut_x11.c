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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "eglutint.h"
#include "eglut_x11.h"

void
_eglutNativeInitDisplay(void)
{
    _eglut->native_dpy = XOpenDisplay(_eglut->display_name);
    if (!_eglut->native_dpy)
        _eglutFatal("failed to initialize native display");

    _eglut->surface_type = EGL_WINDOW_BIT;
}

void
_eglutNativeFiniDisplay(void)
{
    XCloseDisplay(_eglut->native_dpy);
    _eglut->native_dpy = NULL;
}

XIC x11_ic;

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h, const char *icon)
{
    XVisualInfo *visInfo, visTemplate;
    int num_visuals;
    Window root, xwin;
    XSetWindowAttributes attr;
    unsigned long mask;
    EGLint vid;

    if (!eglGetConfigAttrib(_eglut->dpy,
                            win->config, EGL_NATIVE_VISUAL_ID, &vid))
        _eglutFatal("failed to get visual id");

    /* The X window visual must match the EGL config */
    visTemplate.visualid = vid;
    visInfo = XGetVisualInfo(_eglut->native_dpy,
                             VisualIDMask, &visTemplate, &num_visuals);
    if (!visInfo)
        _eglutFatal("failed to get an visual of id 0x%x", vid);

    root = RootWindow(_eglut->native_dpy, DefaultScreen(_eglut->native_dpy));

    /* window attributes */
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(_eglut->native_dpy,
                                    root, visInfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    xwin = XCreateWindow(_eglut->native_dpy, root, x, y, w, h,
                         0, visInfo->depth, InputOutput, visInfo->visual, mask, &attr);
    if (!xwin)
        _eglutFatal("failed to create a window");

    XFree(visInfo);

    /* set hints and properties */
    {
        XSizeHints sizehints;
        sizehints.x = x;
        sizehints.y = y;
        sizehints.width  = w;
        sizehints.height = h;
        if (x == -1 && y == -1) {
            sizehints.win_gravity = CenterGravity;
            sizehints.flags = USSize | PWinGravity;
        } else {
            sizehints.flags = USSize | USPosition;
        }
        XSetNormalHints(_eglut->native_dpy, xwin, &sizehints);
        XSetStandardProperties(_eglut->native_dpy, xwin,
                               title, title, None, (char **) NULL, 0, &sizehints);
    }

    if (icon != NULL) {
        unsigned int img_w, img_h;
        Atom wm_icon = XInternAtom(_eglut->native_dpy, "_NET_WM_ICON", False);
        Atom cardinal = XInternAtom(_eglut->native_dpy, "CARDINAL", False);

        unsigned char* img_data = _eglutReadPNG(icon, &img_w, &img_h);
        if (img_data != NULL) {
            unsigned char* data = malloc((img_w * img_h + 2) * 4);
            *((unsigned int*) &data[0]) = img_w;
            *((unsigned int*) &data[4]) = img_h;
            printf("%i %i\n", img_w, img_h);
            for (ssize_t i = (img_w * img_h - 1) * 4; i >= 0; i -= 4) {
                data[8 + i + 3] = img_data[i + 3];
                data[8 + i + 2] = img_data[i + 0];
                data[8 + i + 1] = img_data[i + 1];
                data[8 + i + 0] = img_data[i + 2];
            }
            XChangeProperty(_eglut->native_dpy, xwin, wm_icon, cardinal, 32, PropModeReplace, data, img_w * img_h + 2);
            //free(data);
        }
        free(img_data);
    }


    XMapWindow(_eglut->native_dpy, xwin);

    win->native.u.window = xwin;
    win->native.width = w;
    win->native.height = h;

    Atom WM_DELETE_WINDOW = XInternAtom(_eglut->native_dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(_eglut->native_dpy, xwin, &WM_DELETE_WINDOW, 1);

    XIM im = XOpenIM(_eglut->native_dpy, NULL, NULL, NULL);
    if (im != NULL) {
        x11_ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, xwin, NULL);
        if (x11_ic != NULL) {
            XSetICFocus(x11_ic);
        }
    }
}

void
_eglutNativeFiniWindow(struct eglut_window *win)
{
    XDestroyWindow(_eglut->native_dpy, win->native.u.window);
}

static int
lookup_keysym(KeySym sym)
{
    int special;

    switch (sym) {
        case XK_F1:
            special = EGLUT_KEY_F1;
            break;
        case XK_F2:
            special = EGLUT_KEY_F2;
            break;
        case XK_F3:
            special = EGLUT_KEY_F3;
            break;
        case XK_F4:
            special = EGLUT_KEY_F4;
            break;
        case XK_F5:
            special = EGLUT_KEY_F5;
            break;
        case XK_F6:
            special = EGLUT_KEY_F6;
            break;
        case XK_F7:
            special = EGLUT_KEY_F7;
            break;
        case XK_F8:
            special = EGLUT_KEY_F8;
            break;
        case XK_F9:
            special = EGLUT_KEY_F9;
            break;
        case XK_F10:
            special = EGLUT_KEY_F10;
            break;
        case XK_F11:
            special = EGLUT_KEY_F11;
            break;
        case XK_F12:
            special = EGLUT_KEY_F12;
            break;
        case XK_KP_Left:
        case XK_Left:
            special = EGLUT_KEY_LEFT;
            break;
        case XK_KP_Up:
        case XK_Up:
            special = EGLUT_KEY_UP;
            break;
        case XK_KP_Right:
        case XK_Right:
            special = EGLUT_KEY_RIGHT;
            break;
        case XK_KP_Down:
        case XK_Down:
            special = EGLUT_KEY_DOWN;
            break;
        default:
            special = -1;
            break;
    }

    return special;
}

static void
next_event(struct eglut_window *win)
{
    int redraw = 0;
    XEvent event, ahead;

    if (!XPending(_eglut->native_dpy)) {
        /* there is an idle callback */
        if (_eglut->idle_cb) {
            _eglut->idle_cb();
            return;
        }

        /* the app requests re-display */
        if (_eglut->redisplay)
            return;
    }

    /* block for next event */
    XNextEvent(_eglut->native_dpy, &event);

    if (XFilterEvent(&event, win->native.u.window)) {
        _eglut->redisplay = redraw;
        return;
    }

    switch (event.type) {
        case Expose:
            redraw = 1;
            break;
        case ConfigureNotify:
            win->native.x = event.xconfigure.x;
            win->native.y = event.xconfigure.y;
            win->native.width = event.xconfigure.border_width;
            win->native.width = event.xconfigure.width;
            win->native.height = event.xconfigure.height;
            if (win->reshape_cb)
                win->reshape_cb(win->native.width, win->native.height);
            break;
        case KeyPress:
        case KeyRelease:
        {
            char buffer[5];
            memset(buffer, 0, 5);
            KeySym sym;
            int r;
            int type;
            if (event.type == KeyPress) {
                r = Xutf8LookupString(x11_ic, (XKeyPressedEvent*) &event, buffer, sizeof(buffer), &sym, NULL);
                type = EGLUT_KEY_PRESS;
            } else {
                r = XLookupString(&event.xkey, buffer, sizeof(buffer), &sym, NULL);
                type = EGLUT_KEY_RELEASE;
            }

            if (event.type == KeyRelease) {
                if (XEventsQueued(_eglut->native_dpy, QueuedAfterReading)) {
                    XPeekEvent(_eglut->native_dpy, &ahead);
                    if (ahead.type == KeyPress &&
                        ahead.xkey.window == event.xkey.window &&
                        ahead.xkey.keycode == event.xkey.keycode &&
                        ahead.xkey.time == event.xkey.time) {
                        type = EGLUT_KEY_REPEAT;
                        XNextEvent(_eglut->native_dpy, &event);
                    }
                }
            }

            if (r > 0 && win->keyboard_cb) {
                win->keyboard_cb(buffer, type);
            }

            if (win->special_cb) {
                /*r = lookup_keysym(sym);
                if (r == -1)*/
                    r = sym;
                if (r >= 0)
                    win->special_cb(r, type);
            }
            if (type != EGLUT_KEY_REPEAT)
                redraw = 1;
            break;
        }
        case MotionNotify:
        {
            if (win->mouse_cb)
                win->mouse_cb(event.xmotion.x, event.xmotion.y);
            break;
        }
        case ButtonPress:
        {
            if (win->mouse_button_cb)
                win->mouse_button_cb(event.xbutton.x, event.xbutton.y, event.xbutton.button, EGLUT_MOUSE_PRESS);
            break;
        }
        case ButtonRelease:
        {
            if (win->mouse_button_cb)
                win->mouse_button_cb(event.xbutton.x, event.xbutton.y, event.xbutton.button, EGLUT_MOUSE_RELEASE);
            break;
        }
        case ClientMessage:
        {
            if ((ulong) event.xclient.data.l[0] == XInternAtom(_eglut->native_dpy, "WM_DELETE_WINDOW", False)) {
                if (win->close_cb) {
                    win->close_cb();
                } else {
                    if (_eglut->current)
                        eglutDestroyWindow(_eglut->current->index);
                    eglutFini();
                }
            }
        }
        default:
            ; /*no-op*/
    }

    _eglut->redisplay = redraw;
}

void
_eglutNativeEventLoop(void)
{
    while (1) {
        struct eglut_window *win = _eglut->current;

        if (_eglut->native_dpy == NULL)
            break;

        next_event(win);

        if (_eglut->redisplay) {
            _eglut->redisplay = 0;

            if (win->display_cb)
                win->display_cb();
            eglSwapBuffers(_eglut->dpy, win->surface);
        }
    }
}

void eglutWarpMousePointer(int x, int y) {
    XWarpPointer(_eglut->native_dpy, None, _eglut->current->native.u.window, 0, 0, 0, 0, x, y);
    XFlush(_eglut->native_dpy);
}

void eglutSetMousePointerVisiblity(int visible) {
    if (visible == EGLUT_POINTER_INVISIBLE) {
        char emptyData[] = {0, 0, 0, 0, 0, 0, 0, 0};
        XColor black;
        black.red = 0;
        black.green = 0;
        black.blue = 0;
        Pixmap emptyBitmap = XCreateBitmapFromData(_eglut->native_dpy, _eglut->current->native.u.window, emptyData, 8, 8);
        Cursor cursor = XCreatePixmapCursor(_eglut->native_dpy, emptyBitmap, emptyBitmap, &black, &black, 0, 0);
        XDefineCursor(_eglut->native_dpy, _eglut->current->native.u.window, cursor);
        XFreeCursor(_eglut->native_dpy, cursor);
        XFreePixmap(_eglut->native_dpy, emptyBitmap);
    } else if (visible == EGLUT_POINTER_VISIBLE) {
        XUndefineCursor(_eglut->native_dpy, _eglut->current->native.u.window);
    }
}

int eglutToggleFullscreen()
{
    // http://stackoverflow.com/questions/10897503/opening-a-fullscreen-opengl-window
    _eglut->window_fullscreen = (_eglut->window_fullscreen == EGLUT_WINDOWED ? EGLUT_FULLSCREEN : EGLUT_WINDOWED);
    Atom wm_state = XInternAtom(_eglut->native_dpy, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(_eglut->native_dpy, "_NET_WM_STATE_FULLSCREEN", False);

    XEvent xev;
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = _eglut->current->native.u.window;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = _eglut->window_fullscreen;
    xev.xclient.data.l[1] = fullscreen;
    xev.xclient.data.l[2] = 0;

    XMapWindow(_eglut->native_dpy, _eglut->current->native.u.window);

    XSendEvent (_eglut->native_dpy, DefaultRootWindow(_eglut->native_dpy), False,
                SubstructureRedirectMask | SubstructureNotifyMask, &xev);

    XFlush(_eglut->native_dpy);
    return -1;
}

Display* eglutGetDisplay() {
    return _eglut->native_dpy;
}

Window eglutGetWindowHandle() {
    return _eglut->current->native.u.window;
}