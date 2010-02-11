include $(top_srcdir)/globals.mak 
SUBDIRS = doc 
DIST_SUBDIRS = doc

# Install my_script in $(bindir) and distribute it.
# FIXME: find out if this is uninstalled by 
dist_bin_SCRIPTS = midistream scripts/scenic

TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 PATH=../$(src)/main:$(PATH) trial
        
TESTS = scenic/test/test_communication.py \
		scenic/test/test_x11.py \
		scenic/test/test_ports.py \
		scenic/test/test_cameras.py \
		scenic/test/test_signals.py

all: 

install-exec-hook: 
	[ -d $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages ] || mkdir -p $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages
	PYTHONPATH=$(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages python setup.py build
	PYTHONPATH=$(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages python setup.py install --prefix=$(DESTDIR)$(prefix)
# install -m 755 -D miville-wrapper $(DESTDIR)$(bindir)/miville
#	install -m 644 -D miville.desktop $(DESTDIR)$(datarootdir)/applications/miville.desktop
#	install -m 644 -D miville.png $(DESTDIR)$(datarootdir)/pixmaps/miville.png
#	if [ -d docs/_build/html ]; then \
#		mkdir -p $(DESTDIR)$(docdir)/miville ; \
#		install -d $(top_srcdir)/py/docs/_build/html $(DESTDIR)$(docdir)/miville/sphinx ; \
#	fi
#	help2man --no-info --include=miville-man.txt --name="The Miville Audio/Video Streaming Software for GNU/Linux" ./mivilled > mivilled.1
#	install -D -m 644 mivilled.1 $(DESTDIR)$(mandir)/man1/mivilled.1
	help2man --no-info --name="The midistream MIDI RTP Streaming Software for GNU/Linux" ./midistream > midistream.1
	install -D -m 644 midistream.1 $(DESTDIR)$(mandir)/man1/midistream.1
#

# FIXME: find out if using dist_bin_SCRIPTS makes any of this redundant 
uninstall-hook:
	awk '/miville/ { system("rm -rf " $1) }' $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages/easy-install.pth 
	sed -i '/miville-.*egg/d' $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages/easy-install.pth 
	rm -rf $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages/scenic-*.egg
	rm -f $(DESTDIR)$(mandir)/man1/midistream.1
#	rm -f $(DESTDIR)$(bindir)/mivilled
#	rm -f $(DESTDIR)$(bindir)/miville
#	rm -f $(DESTDIR)$(bindir)/restart_jackd.py
#	rm -f $(DESTDIR)$(mandir)/man1/mivilled.1
#	rm -f $(DESTDIR)$(datarootdir)/applications/miville.desktop
#	rm -f $(DESTDIR)$(datarootdir)/pixmaps/miville.png

# temporarily removed: check_ping.py
# not much useful:
#        test/test_network.py 
# broken:
#        test/test_com_chan.py 
