include $(top_srcdir)/globals.mak

noinst_LIBRARIES = libtcp.a

libtcp_a_SOURCES = tcpServer.cpp tcpThread.cpp parser.cpp asioSocketServer.cpp \
                   tcpServer.h tcpThread.h parser.h singleBuffer.h\
                   $(inc)/msgThread.h $(inc)/baseThread.h $(inc)/baseModule.h $(inc)/queuePair.h 

libtcp_a_CPPFLAGS = $(AM_CPPFLAGS) $(BOOST_CPPFLAGS)

