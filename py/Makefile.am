include $(top_srcdir)/globals.mak

SUBDIRS = connectors devices maugis protocols streams ui utils

TESTS_ENVIRONMENT = ps aux | grep miville | grep python | awk '{print($2)}' | xargs kill 2> /dev/null; LANG=en_CA.UTF-8 trial  
TESTS = test/test_com_chan.py test/test_addressbook.py \
		test/test_common.py test/test_devices.py test/test_i18n.py \
		test/test_log.py test/test_network.py test/test_observer.py 
#		test/solo_test_telnet_settings.py 


