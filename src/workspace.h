// Copyright (c) 2006 Nigel Tao.
// Licenced under the GNU General Public Licence (GPL) version 2.

#ifndef SUPERSWITCHER_WORKSPACE_H
#define SUPERSWITCHER_WORKSPACE_H

#include <gtk/gtk.h>
#include <libwnck/libwnck.h>

#include "forward_declarations.h"

struct _SSWorkspace {
  SSScreen *        screen;
  WnckWorkspace *   wnck_workspace;
  int               viewport;

  GtkWidget *   widget;
  GtkWidget *   header;
  GtkWidget *   window_container;
  char *        title;

  GList *   windows;
};

SSWorkspace *   ss_workspace_new    (SSScreen *screen, WnckWorkspace *wnck_workspace, int viewport);
void            ss_workspace_free   (SSWorkspace *workspace);

void   ss_workspace_add_window       (SSWorkspace *workspace, SSWindow *window);
void   ss_workspace_remove_window    (SSWorkspace *workspace, SSWindow *window);
void   ss_workspace_reorder_window   (SSWorkspace *workspace, SSWindow *window, int new_index);

int   ss_workspace_find_index_near_point (SSWorkspace *workspace, int x, int y);

#endif
