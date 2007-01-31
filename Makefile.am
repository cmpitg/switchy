SUBDIRS = data po src

EXTRA_DIST = \
	autogen.sh \
	$(addsuffix .in, $(INTLTOOL_BUILT))

INTLTOOL_BUILT = \
	intltool-extract \
	intltool-merge \
	intltool-update
DISTCLEANFILES = $(INTLTOOL_BUILT)
