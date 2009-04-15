include $(top_srcdir)/globals.mak

noinst_LIBRARIES = libtcp.a

libtcp_a_SOURCES = tcpServer.cpp tcpThread.cpp asioThread.cpp \
                   tcpServer.h tcpThread.h s ingleBuffer.h\
                   $(inc)/msgThread.h $(inc)/baseThread.h $(inc)/baseModule.h $(inc)/queuePair.h 

libtcp_a_CPPFLAGS = $(AM_CPPFLAGS) $(BOOST_CPPFLAGS)

