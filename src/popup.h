// Copyright (c) 2006 Nigel Tao.
// Licenced under the GNU General Public Licence (GPL) version 2.

#ifndef SUPERSWITCHER_POPUP_H
#define SUPERSWITCHER_POPUP_H

#include <gtk/gtk.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "screen.h"

typedef struct _Popup Popup;
struct _Popup
{
  SSScreen *    screen;
  GtkWidget *   window;
  GtkWidget *   box_to_add_search_widget;
  GtkWidget *   screen_container;

  gulong   signal_id_active_window_changed;
  gulong   signal_id_active_workspace_changed;
  gulong   signal_id_window_closed;
  gulong   signal_id_window_opened;
  gulong   signal_id_workspace_created;
  gulong   signal_id_workspace_destroyed;

  GtkWidget *   search_widget;
  GtkWidget *   search_text_label;
  GtkWidget *   search_num_matches_label;

  // on_workspace_created hack
  gboolean   owc_complete_action_new_workspace;
  gboolean   owc_also_bring_active_window;
  guint32    owc_time;
};

Popup *   popup_create   (SSScreen *screen);
void      popup_free     (Popup *popup_window);

void   popup_on_key_press   (Popup *popup_window, Display *x_display, XKeyEvent *x_key_event);

#endif
