include $(top_srcdir)/globals.mak

bin_PROGRAMS = dc-ctl

dc_ctl_SOURCES = dc-ctl.cpp
dc_ctl_CPPFLAGS= $(AM_CPPFLAGS) 
LDADD = $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(LIBDC1394_LIBS)
