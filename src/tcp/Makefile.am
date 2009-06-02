include $(top_srcdir)/globals.mak
SUBDIRS = . test
lib_LTLIBRARIES = libtcp.la

libtcp_la_SOURCES = tcpServer.cpp tcpThread.cpp asioThread.cpp tcpServer.h tcpThread.h asioThread.h

libtcp_la_CPPFLAGS = $(AM_CPPFLAGS) $(BOOST_CPPFLAGS) 
