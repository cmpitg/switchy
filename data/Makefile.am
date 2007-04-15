# Dbus service file
servicedir = $(datadir)/dbus-1/services
service_in_files = superswitcher.SuperSwitcher.service.in
service_DATA = $(service_in_files:.service.in=.service)

# Rule to make the service file with bindir expanded
$(service_DATA): $(service_in_files) Makefile
	@sed -e "s|\@bindir\@|$(bindir)|" $< > $@

EXTRA_DIST = \
	$(service_in_files)

DISTCLEANFILES = \
	$(service_DATA) \
	Makefile.in
