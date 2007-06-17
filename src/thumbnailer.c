// Copyright (c) 2006 Nigel Tao.
// Licenced under the GNU General Public Licence (GPL) version 2.

#ifdef HAVE_XCOMPOSITE

#include "thumbnailer.h"

#include "screen.h"
#include "window.h"
#include "workspace.h"
#include "xinerama.h"

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/Xlib.h>

//------------------------------------------------------------------------------

#define THUMBNAIL_SIZE 48

//------------------------------------------------------------------------------

gboolean show_window_thumbnails = FALSE;

//------------------------------------------------------------------------------

gboolean
init_composite (void)
{
  // First, check the Composite extension, then the Render extension.
  int error_base;
  int event_base;
  int version_major;
  int version_minor;

  if (!XCompositeQueryExtension (gdk_display, &event_base, &error_base)) {
    return FALSE;
  }

  // We need at least version 0.2, for XCompositeNameWindowPixmap.
  XCompositeQueryVersion (gdk_display, &version_major, &version_minor);
  if (version_major <= 0 && version_minor < 2) {
    return FALSE;
  }

  if (!XRenderQueryExtension (gdk_display, &event_base, &error_base)) {
    return FALSE;
  }

  // We need at least version 0.6, for XRenderSetPictureTransform.
  XRenderQueryVersion (gdk_display, &version_major, &version_minor);
  if (version_major <= 0 && version_minor < 6) {
    return FALSE;
  }

  XCompositeRedirectSubwindows (gdk_display,
      GDK_WINDOW_XWINDOW (gdk_get_default_root_window ()),
      CompositeRedirectAutomatic);
  return TRUE;
}

//------------------------------------------------------------------------------

gboolean
uninit_composite (void)
{
  XCompositeUnredirectSubwindows (gdk_display,
      GDK_WINDOW_XWINDOW (gdk_get_default_root_window ()),
      CompositeRedirectAutomatic);
  return TRUE;
}

//------------------------------------------------------------------------------

// This code can only run after thumbnailer's drawing_area has been realized,
// and after the main loop has run so that wnck_window is initialized.
static void
initialize_thumbnailer_pictures (SSThumbnailer *thumbnailer)
{
  GdkScreen *screen;
  XRenderPictFormat *format;
  XRenderPictureAttributes pa;

  screen = gtk_widget_get_screen (thumbnailer->drawing_area);
  format = XRenderFindVisualFormat (gdk_display, DefaultVisual (
      gdk_display, gdk_screen_get_number (screen)));

  thumbnailer->thumbnail_pixmap = gdk_pixmap_new (
      thumbnailer->drawing_area->window, THUMBNAIL_SIZE, THUMBNAIL_SIZE, -1);

  thumbnailer->thumbnail_picture = XRenderCreatePicture (gdk_display,
      GDK_DRAWABLE_XID (thumbnailer->thumbnail_pixmap), format, 0, NULL);

  pa.subwindow_mode = IncludeInferiors;
  thumbnailer->window_picture = XRenderCreatePicture (gdk_display,
      wnck_window_get_xid (thumbnailer->wnck_window),
      format, CPSubwindowMode, &pa);
  XRenderSetPictureFilter (gdk_display, thumbnailer->window_picture,
      "good", NULL, 0);
}

//------------------------------------------------------------------------------

static gboolean
on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  GdkGC *gc;
  XTransform transform;
  double scale;
  int wx, wy, ww, wh;
  int frame_left, frame_right, frame_top, frame_bottom;
  int thumbnail_width, thumbnail_height;
  int offset_x, offset_y;
  SSWindow *window;
  SSThumbnailer *thumbnailer;
  thumbnailer = (SSThumbnailer *) data;
  window = thumbnailer->window;

  if (thumbnailer->thumbnail_pixmap == NULL) {
    initialize_thumbnailer_pictures (thumbnailer);
  }
  gc = gdk_gc_new (thumbnailer->thumbnail_pixmap);

  ss_xinerama_get_frame_extents (
      window->workspace->screen->xinerama, window,
      &frame_left, &frame_right, &frame_top, &frame_bottom);

  wnck_window_get_geometry (window->wnck_window, &wx, &wy, &ww, &wh);

  scale = ww > wh ? ww : wh;
  scale /= (double) THUMBNAIL_SIZE;
  thumbnail_width = thumbnail_height = THUMBNAIL_SIZE;
  if (ww > wh) {
    thumbnail_height = wh * THUMBNAIL_SIZE / ww;
  } else {
    thumbnail_width = ww * THUMBNAIL_SIZE / wh;
  }
  
  transform.matrix[0][0] = XDoubleToFixed (scale);
  transform.matrix[0][1] = XDoubleToFixed (0.0);
  transform.matrix[0][2] = XDoubleToFixed (-frame_left * (scale - 1.0));
  transform.matrix[1][0] = XDoubleToFixed (0.0);
  transform.matrix[1][1] = XDoubleToFixed (scale);
  transform.matrix[1][2] = XDoubleToFixed (-frame_top * (scale - 1.0));
  transform.matrix[2][0] = XDoubleToFixed (0.0);
  transform.matrix[2][1] = XDoubleToFixed (0.0);
  transform.matrix[2][2] = XDoubleToFixed (1.0);
  XRenderSetPictureTransform (gdk_display, thumbnailer->window_picture,
      &transform);

  XRenderComposite (gdk_display, PictOpSrc,
    thumbnailer->window_picture, None, thumbnailer->thumbnail_picture,
    0, 0, 0, 0, 0, 0, THUMBNAIL_SIZE, THUMBNAIL_SIZE);

  offset_x = widget->allocation.x + (THUMBNAIL_SIZE - thumbnail_width) / 2;
  offset_y = widget->allocation.y + (THUMBNAIL_SIZE - thumbnail_height) / 2;

  gdk_draw_drawable (thumbnailer->drawing_area->window, gc,
      thumbnailer->thumbnail_pixmap, 0, 0,
      offset_x, offset_y, thumbnail_width, thumbnail_height);

  gdk_draw_rectangle (thumbnailer->drawing_area->window,
      thumbnailer->drawing_area->style->black_gc, FALSE,
      offset_x, offset_y, thumbnail_width - 1, thumbnail_height - 1);

  g_object_unref (gc);
  return FALSE;
}

//------------------------------------------------------------------------------

SSThumbnailer *
ss_thumbnailer_new (SSWindow *window, WnckWindow *wnck_window, GtkWidget *drawing_area)
{
  SSThumbnailer *t;

  gtk_widget_set_size_request (drawing_area, THUMBNAIL_SIZE, THUMBNAIL_SIZE);
  gtk_widget_set_app_paintable (drawing_area, TRUE);

  t = g_new (SSThumbnailer, 1);
  t->window = window;
  t->wnck_window = wnck_window;
  t->drawing_area = drawing_area;
  t->thumbnail_pixmap = NULL;
  t->thumbnail_picture = None;
  t->window_picture = None;

  g_signal_connect (G_OBJECT (drawing_area), "expose-event",
                    G_CALLBACK (on_expose_event),
                    t);
  return t;
}

//------------------------------------------------------------------------------

void
ss_thumbnailer_free (SSThumbnailer *thumbnailer)
{
  if (thumbnailer == NULL) {
    return;
  }

  if (thumbnailer->thumbnail_pixmap != NULL) {
    g_object_unref (thumbnailer->thumbnail_pixmap);
    thumbnailer->thumbnail_pixmap = NULL;
  }
  if (thumbnailer->thumbnail_picture != None) {
    XRenderFreePicture (gdk_display, thumbnailer->thumbnail_picture);
    thumbnailer->thumbnail_picture = None;
  }
  if (thumbnailer->window_picture != None) {
    XRenderFreePicture (gdk_display, thumbnailer->window_picture);
    thumbnailer->window_picture = None;
  }

  g_free (thumbnailer);
}

#endif  // #ifdef HAVE_XCOMPOSITE

