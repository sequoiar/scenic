PREFIX=$(prefix)
prefix=$(PREFIX)
lang = en_US fr_CA
progname=maugis

all:

install:
	install -m 755 $(progname).py $(prefix)/bin/$(progname).py
	install -m 755 $(progname).sh $(prefix)/bin/$(progname)
	mkdir -p  -m 755 /usr/share/$(progname)
	install -m 644 $(progname).glade /usr/share/$(progname)

locale: 
	# update the pot files
	@intltool-extract --type=gettext/glade $(progname).glade
	@xgettext --language=Python --keyword=_ --keyword=N_ --output=$(progname).pot $(progname).py $(progname).glade.h
	# apply to existing po files, and make po out of them
	@for language in en_US fr_CA; do \
		msgmerge locale/$$language/LC_MESSAGES/$$language.po $(progname).pot -U; \
		msgfmt locale/$$language/LC_MESSAGES/$$language.po -o locale/$$language/LC_MESSAGES/$$language.mo; \
	done;

.PHONY: locale

