#FIXME: this is a hack because trial goes into the directory first
TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 PATH=../:$(PATH) trial

TESTS = test/test_jack_info.py

noinst_PYTHON = test/test_jack_info.py
bin_PROGRAMS = jack-info
jack_info_SOURCES = jack_info.c
jack_info_CFLAGS= $(JACK_CFLAGS)
LDADD = $(JACK_LIBS)

clean-local:
	rm -rf _trial_temp
	rm -rf test/*.pyc
