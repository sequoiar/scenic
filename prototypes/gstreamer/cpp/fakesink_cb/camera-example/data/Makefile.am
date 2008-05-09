desktopdir = $(datadir)/applications/hildon
desktop_DATA = camera.desktop

EXTRA_DIST = $(desktop_DATA)

edit = sed \
 	-e 's,@datadir\@,$(datadir),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@VERSION\@,$(VERSION),g'

camera.desktop: camera.desktop.in
	@$(edit) $< > $@

CLEANFILES = $(desktop_DATA)
