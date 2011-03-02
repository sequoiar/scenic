#FIXME: this is a hack because trial goes into the directory first
TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 PATH=../:$(PATH) trial

TESTS = test/test_dc_ctl.py

noinst_PYTHON = test/test_dc_ctl.py
bin_PROGRAMS = dc-ctl
dc_ctl_SOURCES = dc-ctl.cpp
dc_ctl_CPPFLAGS= $(AM_CPPFLAGS) 
LDADD = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(LIBDC1394_LIBS) $(LIBRAW1394_LIBS)

clean-local:
	rm -rf _trial_temp
	rm -rf test/*.pyc
