// Copyright (c) 2006 Nigel Tao.
// Licenced under the GNU General Public Licence (GPL) version 2.

#include <gdk/gdk.h>
#include <gdk/gdkwindow.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <X11/keysym.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "screen.h"
#include "popup.h"

//------------------------------------------------------------------------------

static Display *x_display = NULL;
static SSScreen *screen = NULL;
static Popup *popup = NULL;
static int popup_keycode_to_free = -1;

//------------------------------------------------------------------------------

static GdkFilterReturn
filter_func (GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
  XEvent *x_event;
  x_event = (XEvent *) gdk_xevent;

  switch (x_event->type) {
  case KeyPress:
    if (popup_keycode_to_free == -1) {
      popup_keycode_to_free = x_event->xkey.keycode;
      popup = popup_create (screen);
    } else {
      popup_on_key_press (popup, x_display, &x_event->xkey);
    }
    break;
  case KeyRelease:
    if (popup_keycode_to_free == x_event->xkey.keycode) {
      popup_keycode_to_free = -1;
      popup_free (popup);
      popup = NULL;
    }
    break;
  default:
    // No-op.
    break;
  }

  return GDK_FILTER_CONTINUE;
}

//------------------------------------------------------------------------------

static void
grab (Display *x_display, GdkWindow *gdk_window, int keyval)
{
  XGrabKey (x_display,
            XKeysymToKeycode (x_display, keyval),
            AnyModifier,
            GDK_WINDOW_XWINDOW (gdk_window),
            False,
            GrabModeAsync,
            GrabModeAsync);
}

//------------------------------------------------------------------------------

int
main (int argc, char **argv)
{
  GdkWindow *root;

  gtk_init (&argc, &argv);

  root = gdk_get_default_root_window ();
  x_display = GDK_WINDOW_XDISPLAY (root);

  gdk_window_add_filter (root, filter_func, NULL);
  grab (x_display, root, XK_Super_L);
  grab (x_display, root, XK_Super_R);

  screen = ss_screen_new (wnck_screen_get_default (), x_display);

printf ("-------\nSuperSwitcher version 0.4\n"); // TODO: DELETE ME
  gtk_main ();
  return 0;
}
