include $(top_srcdir)/globals.mak 
SUBDIRS = docs prism
DIST_SUBDIRS = miville maugis docs prism

TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 trial
        
all: 

TESTS = test/test_addressbook.py \
        test/test_commands.py \
        test/test_common.py \
        test/test_devices.py \
        test/test_devices_settings.py \
        test/test_i18n.py \
        test/test_streams_conf.py \
        test/check_cli_streams.py \
        test/test_log.py \
        test/test_observer.py \
        test/test_serialize.py \
        test/check_cli_ui.py \
        test/check_devices.py \
        test/check_ping.py \
        test/check_miville_starts.py

#        test/check_milhouse_ipcp.py \
#        test/check_milhouse_ipcp_sync.py \
#EXTRA_DIST: $(shell `../utils/list_miville_files`)

install-exec-hook: 
		[ -d $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages ] || mkdir -p $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages
		PYTHONPATH=$(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages python setup.py build
		PYTHONPATH=$(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages python setup.py install --prefix=$(DESTDIR)$(prefix)
		install -m 755 -D miville-wrapper $(DESTDIR)$(bindir)/miville
		install -m 644 -D miville.desktop $(DESTDIR)$(datarootdir)/applications/miville.desktop
		install -m 644 -D miville.png $(DESTDIR)$(datarootdir)/pixmaps/miville.png
		if [ -d docs/_build/html ]; then \
			mkdir -p $(DESTDIR)$(docdir)/miville ; \
			install -d $(top_srcdir)/py/docs/_build/html $(DESTDIR)$(docdir)/miville/sphinx ; \
		fi
		help2man -N -i miville-man.txt -n "The Miville Audio/Video Streaming Software for GNU/Linux" ./mivilled > mivilled.1
		install -D -m 644 mivilled.1 $(DESTDIR)$(mandir)/man1/mivilled.1
#
# THIS IS BROKEN !! FIXME

uninstall-hook:
		awk '/miville/ { system("rm -rf " $1) }' $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages/easy-install.pth 
		sed -i '/miville-.*egg/d' $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages/easy-install.pth 
		rm -rf $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages/miville-*.egg
		rm -f $(DESTDIR)$(bindir)/mivilled
		rm -f $(DESTDIR)$(bindir)/miville
		rm -f $(DESTDIR)$(bindir)/restart_jackd.py
		rm -f $(DESTDIR)$(mandir)/man1/mivilled.1
		rm -f $(DESTDIR)$(datarootdir)/applications/miville.desktop
		rm -f $(DESTDIR)$(datarootdir)/pixmaps/miville.png

# temporarily removed: check_ping.py
# not much useful:
#        test/test_network.py 
# broken:
#        test/test_com_chan.py 
