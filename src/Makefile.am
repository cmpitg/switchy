# The main binary
bin_PROGRAMS = superswitcher

superswitcher_SOURCES = \
  draganddrop.c \
  draganddrop.h \
  forward_declarations.h \
  popup.c \
  popup.h \
  screen.c \
  screen.h \
  superswitcher.c \
  window.c \
  window.h \
  workspace.c \
  workspace.h \
  xinerama.c \
  xinerama.h

AM_CPPFLAGS = \
  $(SUPERSWITCHER_CFLAGS) \
  -DLOCALEDIR=\""$(datadir)/locale"\" \
  -DSYSCONFDIR=\""$(sysconfdir)"\" \
  -DDATADIR=\""$(datadir)"\" \
  -DG_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DGTK_DISABLE_DEPRECATED \
  -DWNCK_I_KNOW_THIS_IS_UNSTABLE

AM_CFLAGS = @WARN_CFLAGS@

LDADD = ${SUPERSWITCHER_LIBS}
