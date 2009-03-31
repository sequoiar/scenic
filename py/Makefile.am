include $(top_srcdir)/globals.mak 

SUBDIRS = miville maugis 

TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 trial

TESTS = test/test_com_chan.py \
        test/test_addressbook.py \
        test/test_devices.py \
        test/test_i18n.py \
        test/test_log.py \
        test/test_observer.py \
        test/test_telnet_milhouse.py


# to update to new path
#        test/test_common.py 
#
# broken telnet executable: 
#        test/check_ping.py 
#        test/solo_test_preset_settings.py 
#        test/solo_test_telnet_settings.py 
#
# not much useful:
#        test/test_network.py 
#
# 
#     * test_ :  unit tests.
#     * once_ :  work in progress. Not part of official test suites.
#     * check_ : tests that internally start miville and telnet processes 
#                to test. They use py/utils/telnet_testing.py
#     * dist_ :  Official test case testing. Those tests need two computers. 
#                Miville needs to be started manually on both prior to run them. 
