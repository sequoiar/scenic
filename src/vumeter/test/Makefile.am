TESTS_ENVIRONMENT = LANG=en_CA.UTF-8 PATH=../:$(PATH) trial
check_PROGRAMS = meter
TESTS = test_vumeter.py

meter_SOURCES = main.c
meter_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS) $(GST_CFLAGS)
meter_LDADD = $(GLIB_LIBS) $(GTK_LIBS) $(GST_LIBS) $(top_srcdir)/src/vumeter/libvumeter.la
INCLUDES = -I$(top_srcdir)/src/include
