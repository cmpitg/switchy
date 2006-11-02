// Copyright (c) 2006 Nigel Tao.
// Licenced under the GNU General Public Licence (GPL) version 2.

#ifndef SUPERSWITCHER_XINERAMA_H
#define SUPERSWITCHER_XINERAMA_H

#include <X11/Xlib.h>

#include "forward_declarations.h"

struct _SSXineramaScreen {
  int   x;
  int   y;
  int   width;
  int   height;
};

struct _SSXinerama {
  Display *            x_display;
  Window               x_root_window;
  int                  num_screens;
  SSXineramaScreen *   screens;
  int                  minimum_width;
};

SSXinerama *   ss_xinerama_new   (Display *x_display, Window x_root_window);

void   ss_xinerama_move_to_next_screen   (SSXinerama *xinerama, SSWindow *window);

#endif
