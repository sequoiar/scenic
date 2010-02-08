include $(top_srcdir)/globals.mak 

#FIXME: this is a hack because trial goes into the directory first
TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 PATH=$(top_srcdir)/maugis:$(PATH) trial

TESTS = scenic/test/test_communication.py \
		scenic/tests/test_x11.py \
		scenic/tests/test_ports.py \
		scenic/tests/test_signals.py

