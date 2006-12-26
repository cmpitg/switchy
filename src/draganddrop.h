// Copyright (c) 2006 Nigel Tao.
// Licenced under the GNU General Public Licence (GPL) version 2.

#ifndef SUPERSWITCHER_DRAGANDDROP_H
#define SUPERSWITCHER_DRAGANDDROP_H

#include <glib.h>
#include <gtk/gtk.h>

#include "forward_declarations.h"

struct _SSDragAndDrop {
  SSScreen *      screen;

  gboolean        is_dragging;

  int             drag_start_x;
  int             drag_start_y;
  SSWindow *      drag_start_window;
  SSWorkspace *   drag_start_workspace;
  GtkWidget *     drag_start_widget;

  int             drag_x;
  int             drag_y;
  SSWorkspace *   drag_workspace;
  int             new_window_index;
};

SSDragAndDrop *   ss_draganddrop_new    (SSScreen *screen);
void              ss_draganddrop_free   (SSDragAndDrop *dnd);

void   ss_draganddrop_on_motion    (SSDragAndDrop *dnd);
void   ss_draganddrop_on_release   (SSDragAndDrop *dnd);

void   ss_draganddrop_start   (SSDragAndDrop *dnd, SSWindow *window, SSWorkspace *workspace);

#endif
