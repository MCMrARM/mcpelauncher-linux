#ifndef EGLUT_X11_H
#define EGLUT_X11_H

#include <X11/Xlib.h>

Display* eglutGetDisplay();
Window eglutGetWindowHandle();

#endif /* EGLUT_H */