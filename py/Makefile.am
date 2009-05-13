include $(top_srcdir)/globals.mak 

DIST_SUBDIRS = miville maugis docs

TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 trial

TESTS = test/test_addressbook.py \
        test/test_commands.py \
        test/test_common.py \
        test/check_ping.py \
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
		mkdir -p $(DESTDIR)$(prefix)/lib/python2.5/miville
		cp -fvr miville/ $(DESTDIR)$(prefix)/lib/python2.5
		mkdir -p $(DESTDIR)$(prefix)/bin
		cp -fv miville.py $(DESTDIR)$(prefix)/bin/miville

uninstall-hook:
		rm -rf $(DESTDIR)$(prefix)/lib/python2.5/miville
		rm -rf $(DESTDIR)$(prefix)/bin/miville

# temporarily removed: check_ping.py
# not much useful:
#        test/test_network.py 
# broken:
#        test/test_com_chan.py 
