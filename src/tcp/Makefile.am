include $(top_srcdir)/globals.mak

noinst_LIBRARIES = libtcp.a

libtcp_a_SOURCES = tcpServer.cpp tcpThread.cpp asioThread.cpp tcpServer.h tcpThread.h asioThread.h

libtcp_a_CPPFLAGS = $(AM_CPPFLAGS) $(BOOST_CPPFLAGS) 
CXXFLAGS += -Wno-effc++
