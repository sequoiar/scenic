TESTS_ENVIRONMENT = LANG=C PATH=../:$(PATH) trial
check_PROGRAMS = meter
TESTS = test_vumeter.py
noinst_PYTHON = test_vumeter.py
clean-local:
	rm -rf *.pyc
	rm -rf _trial_temp

meter_SOURCES = main.c $(top_builddir)/src/vumeter/vumeter.cpp $(top_builddir)/src/include/vumeter.h

meter_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(GST_CFLAGS)
meter_LDADD = $(GLIB_LIBS) $(GTK_LIBS) $(GST_LIBS) $(top_builddir)/src/vumeter/libvumeter.la
INCLUDES = -I$(top_srcdir)/src/include
