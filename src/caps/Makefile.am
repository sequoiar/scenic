pkglib_LTLIBRARIES = libcaps.la

libcaps_la_SOURCES = CapsServer.cpp CapsServer.h CapsClient.cpp CapsClient.h MulticastCapsServer.cpp MulticastCapsServer.h \
                     capsParser.cpp caps.h capsParser.h 

libcaps_la_CPPFLAGS = $(BOOST_CPPFLAGS) $(GTK_CFLAGS) -I$(top_srcdir)/src/include -I$(top_srcdir)/src

BUILT_SOURCES = caps.cpp
nodist_libcaps_la_SOURCES = $(BUILT_SOURCES)

caps.cpp: $(top_srcdir)/utils/make_caps.py
	$< $@

CLEANFILES = $(BUILT_SOURCES)
