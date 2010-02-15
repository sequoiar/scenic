include $(top_srcdir)/globals.mak 
SUBDIRS = doc scenic scripts rtpmidi

TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 PATH=../../src/main:$(PATH) PYTHONPATH=../../py:$(PYTHONPATH) trial
        
TESTS = scenic/test/test_communication.py \
		scenic/test/test_x11.py \
		scenic/test/test_ports.py \
		scenic/test/test_cameras.py \
		scenic/test/test_signals.py

EXTRA_DIST=scenic.svg

clean-local:
	rm -rf _trial_temp

#all: 
#	epydoc scenic
#	convert -geometry 48x48 -background none scenic.svg scenic.png

#install-exec-hook: 
#	[ -d $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages ] || mkdir -p $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages
#	PYTHONPATH=$(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages python setup.py build
#	PYTHONPATH=$(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages python setup.py install --prefix=$(DESTDIR)$(prefix)

#	install -m 644 -D scenic.desktop $(DESTDIR)$(datarootdir)/applications/scenic.desktop
#	install -m 644 -D scenic.png $(DESTDIR)$(datarootdir)/pixmaps/scenic.png
#	if [ -d docs/_build/html ]; then \
#		mkdir -p $(DESTDIR)$(docdir)/scenic ; \
#		install -d $(top_srcdir)/py/html $(DESTDIR)$(docdir)/scenic/html-documentation ; \
#	fi
#	help2man --no-info --include=scenic-man.txt --name="The Scenic Audio/Video Streaming Software for GNU/Linux" ./scripts/scenic > scenic.1
#	install -D -m 644 scenic.1 $(DESTDIR)$(mandir)/man1/scenic.1
#	help2man --no-info --name="The midistream MIDI RTP Streaming Software for GNU/Linux" ./midistream > midistream.1
#	install -D -m 644 midistream.1 $(DESTDIR)$(mandir)/man1/midistream.1

uninstall-hook:
	rm -f $(DESTDIR)$(mandir)/man1/midistream.1
	rm -f $(DESTDIR)$(bindir)/scenic
	rm -f $(DESTDIR)$(mandir)/man1/scenic.1
	rm -f $(DESTDIR)$(datarootdir)/applications/scenic.desktop
	rm -f $(DESTDIR)$(datarootdir)/pixmaps/scenic.png
