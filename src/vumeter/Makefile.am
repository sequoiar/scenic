SUBDIRS = test

pkglib_LTLIBRARIES = libvumeter.la
libvumeter_la_SOURCES = vumeter.cpp $(top_srcdir)/src/include/vumeter.h
INCLUDES = -I$(top_srcdir)/src/include
libvumeter_la_CPPFLAGS = $(GLIB_CFLAGS) $(GTK_CFLAGS)
