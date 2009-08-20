include $(top_srcdir)/globals.mak 
SUBDIRS = docs
DIST_SUBDIRS = miville maugis docs

TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 trial
        
all: miville.webapp

miville.webapp:
	@zip miville.webapp webapp.ini localstore.json
	
TESTS = test/test_addressbook.py \
        test/test_commands.py \
        test/test_common.py \
        test/test_devices.py \
        test/test_devices_settings.py \
        test/test_i18n.py \
        test/test_log.py \
        test/test_observer.py \
        test/check_cli_ui.py \
        test/check_milhouse_ipcp.py \
        test/check_milhouse_ipcp_sync.py \
        test/check_devices.py \
        test/test_conf.py \
        test/check_miville_starts.py \
        test/check_miville_settings.py

#EXTRA_DIST: $(shell `../utils/list_miville_files`)

install-exec-hook: 
		[ -d $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages ] || mkdir -p $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages
		PYTHONPATH=$(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages python setup.py build
		PYTHONPATH=$(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages python setup.py install --prefix=$(DESTDIR)$(prefix)
		install -m 755 -D miville-wrapper $(DESTDIR)$(prefix)/bin/miville
		install -m 644 -D miville.webapp $(DESTDIR)$(prefix)/share/prism/apps/miville.webapp
		install -m 644 -D miville.desktop $(DESTDIR)$(prefix)/share/applications/miville.desktop
		install -m 644 -D miville.png $(DESTDIR)$(prefix)/share/pixmaps/miville.png
		if [ -d docs/_build/html ]; then \
			mkdir -p $(DESTDIR)$(prefix)/share/doc/miville ; \
			install -d $(top_srcdir)/py/docs/_build/html $(DESTDIR)$(prefix)/share/doc/miville/sphinx ; \
		fi
# help2man -N -i MANPAGE.txt -n "The Miville Audio/Video Streaming Software for GNU/Linux" mivilled > mivilled.1
# install -D -m 644 mivilled.1 $(DESTDIR)$(prefix)/share/man/man1/mivilled.1
#
# THIS IS BROKEN !! FIXME

uninstall-hook:
		awk '/miville/ { system("rm -rf " $1) }' $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages/easy-install.pth 
		sed -i '/miville-.*egg/d' $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages/easy-install.pth 
		rm -rf $(DESTDIR)$(prefix)/lib/$(PYTHON_LIB)/site-packages/miville-*.egg
		rm -f $(DESTDIR)$(prefix)/bin/mivilled
		rm -f $(DESTDIR)$(prefix)/bin/miville
		rm -f $(DESTDIR)$(prefix)/bin/restart_jackd.py
		rm -rf $(DESTDIR)$(prefix)/share/doc/miville/sphinx
		rm -f $(DESTDIR)$(prefix)/share/man/man1/mivilled.1
		rm -f $(DESTDIR)$(prefix)/share/applications/miville.desktop
		rm -f $(DESTDIR)$(prefix)/share/pixmaps/miville.png

# temporarily removed: check_ping.py
# not much useful:
#        test/test_network.py 
# broken:
#        test/test_com_chan.py 
