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

#define VERSION_STRING "0.4"

//------------------------------------------------------------------------------

static Display *x_display = NULL;
static Window x_root_window = None;
static SSScreen *screen = NULL;
static Popup *popup = NULL;
static int popup_keycode_to_free = -1;
static gboolean trigger_on_caps_lock = FALSE;
static gboolean show_version_and_exit = FALSE;

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
grab (int keyval)
{
  XGrabKey (x_display,
            XKeysymToKeycode (x_display, keyval),
            AnyModifier,
            x_root_window,
            False,
            GrabModeAsync,
            GrabModeAsync);
}

//------------------------------------------------------------------------------

static void
disable_caps_lock_default_behavior ()
{
  KeyCode keycode;
  XModifierKeymap *map;
	char *error_msg;

  keycode = XKeysymToKeycode (x_display, XK_Caps_Lock);

  map = XGetModifierMapping (x_display);
  map = XDeleteModifiermapEntry (map, keycode, LockMapIndex);

  error_msg = NULL;
  switch (XSetModifierMapping (x_display, map)) {
    case MappingSuccess:
      break;
    case MappingBusy:
      error_msg = "since it's busy.";
      break;
    default:
      error_msg = "for some unknown reason.";
      break;
  }
  if (error_msg != NULL) {
    fprintf (stderr,
             "SuperSwitcher error - could not disable the Caps Lock key, %s\n",
             error_msg);
  }
  XFreeModifiermap (map);
}

//------------------------------------------------------------------------------

int
main (int argc, char **argv)
{
  static const GOptionEntry options[] = {
    { "trigger-on-caps-lock", 'c', 0, G_OPTION_ARG_NONE, &trigger_on_caps_lock,
      "Make the Caps Lock key also switch windows", NULL },
    { "version", 'v', 0, G_OPTION_ARG_NONE, &show_version_and_exit,
      "Show the version number and exit", NULL },
    { NULL }
  };

  GdkWindow *root;
  GOptionContext *context;
  GError *error;

  gtk_init (&argc, &argv);

  context = g_option_context_new ("");
  error = NULL;
  g_option_context_add_main_entries (context, options, NULL);
  g_option_context_parse (context, &argc, &argv, &error);

  if (show_version_and_exit) {
    printf ("SuperSwitcher version %s\n", VERSION_STRING);
    return 0;
  }

  root = gdk_get_default_root_window ();
  x_display = GDK_WINDOW_XDISPLAY (root);
  x_root_window = GDK_WINDOW_XWINDOW (root);

  gdk_window_add_filter (root, filter_func, NULL);
  grab (XK_Super_L);
  grab (XK_Super_R);
  if (trigger_on_caps_lock) {
    disable_caps_lock_default_behavior ();
    grab (XK_Caps_Lock);
  }

  screen = ss_screen_new (wnck_screen_get_default (), x_display, x_root_window);

  gtk_main ();
  return 0;
}
