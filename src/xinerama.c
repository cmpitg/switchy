// Copyright (c) 2006 Nigel Tao.
// Licenced under the GNU General Public Licence (GPL) version 2.

#include "xinerama.h"

#include <gdk/gdk.h>
#include <glib.h>
#include <libwnck/libwnck.h>

#ifdef HAVE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include "window.h"

//------------------------------------------------------------------------------

SSXinerama *
ss_xinerama_new (Display *x_display)
{
  int num_screens;
  int minimum_width;
  SSXineramaScreen *screens;
  SSXinerama *xinerama;

  minimum_width = 0;

#ifdef HAVE_XINERAMA
  gboolean xinerama_is_active = XineramaIsActive (x_display);
#else
  gboolean xinerama_is_active = FALSE;
#endif

  if (xinerama_is_active) {
#ifdef HAVE_XINERAMA
    XineramaScreenInfo *xsi_array =
      XineramaQueryScreens(x_display, &num_screens);
    screens = g_new (SSXineramaScreen, num_screens);

    int i;
    for (i = 0; i < num_screens; i++) {
      XineramaScreenInfo *xsi = &xsi_array[i];
      screens[i].x = xsi->x_org;
      screens[i].y = xsi->y_org;
      screens[i].width  = xsi->width;
      screens[i].height = xsi->height;
      if (i == 0) {
        minimum_width = screens[0].width;
      } else {
        minimum_width = MIN (minimum_width, screens[i].width);
      }
    }
    XFree (xsi_array);
#endif
  } else {
    num_screens = 1;
    int x_screen = DefaultScreen (x_display);
    screens = g_new (SSXineramaScreen, num_screens);
    screens[0].x = 0;
    screens[0].y = 0;
    screens[0].width  = DisplayWidth (x_display, x_screen);
    screens[0].height = DisplayHeight (x_display, x_screen);
    minimum_width = screens[0].width;
  }

  xinerama = g_new (SSXinerama, 1);
  xinerama->x_display = x_display;
  xinerama->num_screens = num_screens;
  xinerama->screens = screens;
  xinerama->minimum_width = minimum_width;
  return xinerama;
}

//------------------------------------------------------------------------------

static Window get_x_parent_window (Display *x_display, Window x_window)
{
  Window root, parent;
  Window *children;
  unsigned int num_children;

  XQueryTree (x_display, x_window, &root, &parent, &children, &num_children);
  XFree (children);
  return parent;
}

//------------------------------------------------------------------------------

static int get_best_screen (SSXinerama *xinerama, SSWindow *window)
{
  GdkRectangle r;
  SSXineramaScreen *xs;
  int i;
  int rightmost_left;
  int leftmost_right;
  int bottommost_top;
  int topmost_bottom;
  int dx, dy;
  int overlapping_area;
  int best_overlapping_area;
  int best_screen;

  best_overlapping_area = 0;
  best_screen = 0;

  // first, find out which Xinerama screen the window is most on
  wnck_window_get_geometry (window->wnck_window, &r.x, &r.y, &r.width, &r.height);
  for (i = 0; i < xinerama->num_screens; i++) {
    xs = &xinerama->screens[i];
    rightmost_left = MAX (xs->x, r.x);
    leftmost_right = MIN (xs->x + xs->width, r.x + r.width);
    bottommost_top = MAX (xs->y, r.y);
    topmost_bottom = MIN (xs->y + xs->height, r.y + r.height);

    dx = leftmost_right - rightmost_left;
    dy = topmost_bottom - bottommost_top;
    if (dx < 0 || dy < 0) {
      overlapping_area = 0;
    } else {
      overlapping_area = dx * dy;
    }

    if (i == 0 || overlapping_area > best_overlapping_area) {
      best_overlapping_area = overlapping_area;
      best_screen = i;
    }
  }

  return best_screen;
}

//------------------------------------------------------------------------------

void ss_xinerama_move_to_next_screen (SSXinerama *xinerama, SSWindow *window)
{
  int best_screen;
  SSXineramaScreen *xs;
  int dx, dy;
  Window x_window, x_parent_window;
  gboolean is_maximized;

  // Xlib callback junk
  Window root;
  int x, y;
  unsigned int width, height, border_width, depth;

  if (xinerama->num_screens == 1) {
    return;
  }
  if (window == NULL) {
    return;
  }

  // In metacity, you can't move a maximized window with XMoveWindow.  Our
  // gross hack is to temporarily unmaximize, move, and re-maximize.
  is_maximized = wnck_window_is_maximized (window->wnck_window);
  if (is_maximized) {
    wnck_window_unmaximize (window->wnck_window);
  }

  best_screen = get_best_screen (xinerama, window);
  // libwnck doesn't have a set_geometry function, nor does it give the
  // geometry of the window manager's frame, so we do this with X calls.
  x_window = wnck_window_get_xid (window->wnck_window);
  x_parent_window = get_x_parent_window (xinerama->x_display, x_window);
  XGetGeometry (xinerama->x_display, x_parent_window,
    &root, &x, &y, &width, &height, &border_width, &depth);

  // now move to the next screen.
  xs = &xinerama->screens[best_screen];
  dx = x - xs->x;
  dy = y - xs->y;
  best_screen = (best_screen + 1) % xinerama->num_screens;
  xs = &xinerama->screens[best_screen];

  // Yuck.  So, I move the window to the desired x,y of its PARENT window on
  // the other (Xinerama) screen.  This seems unintuitive, but it's what works
  // with metacity.  This may be totally broken on other window managers.
  XMoveWindow (xinerama->x_display, x_window, xs->x + dx, xs->y + dy);

  if (is_maximized) {
    wnck_window_maximize (window->wnck_window);
  }
}
