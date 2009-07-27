include $(top_srcdir)/globals.mak 
SUBDIRS = docs
DIST_SUBDIRS = miville maugis docs

TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 trial
        
	
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
        test/check_miville_starts.py \
        test/check_miville_settings.py

#EXTRA_DIST: $(shell `../utils/list_miville_files`)

install-exec-hook:
		PYTHONPATH=$(DESTDIR)$(prefix)/lib/python2.5/site-packages python setup.py build
		PYTHONPATH=$(DESTDIR)$(prefix)/lib/python2.5/site-packages python setup.py install --prefix=$(DESTDIR)$(prefix)

		if [ -d docs/_build/html ]; then \
			mkdir -p $(DESTDIR)$(prefix)/share/doc/miville ; \
			install -d $(top_srcdir)/py/docs/_build/html $(DESTDIR)$(prefix)/share/doc/miville/sphinx ; \
		fi

uninstall-hook:
		awk '/miville/ { system("rm -rf " $1) }' $(DESTDIR)$(prefix)/lib/python2.5/site-packages/easy-install.pth 
		sed -i '/miville-.*egg/d' $(DESTDIR)$(prefix)/lib/python2.5/site-packages/easy-install.pth 
		rm -rf $(DESTDIR)$(prefix)/bin/mivilled
		rm -rf $(DESTDIR)$(prefix)/bin/restart_jackd.py
		rm -rf $(DESTDIR)$(prefix)/share/doc/miville/sphinx

# temporarily removed: check_ping.py
# not much useful:
#        test/test_network.py 
# broken:
#        test/test_com_chan.py 
