include $(top_srcdir)/src/globals.mak

bin_PROGRAMS = mainTester 

mainTester_SOURCES = msgThreadFactory.cpp gstThread.cpp gstSenderThread.cpp gstReceiverThread.cpp main.cpp 


LDADD = $(GLIB_LIBS) $(ALL_GST_LIBS) $(JACK_LIBS) $(src)/tcp/libtcp.a $(src)/gst/libgst.a  $(LIBLO_LIBS) $(GTK_LIBS) $(LIBUTIL) $(LIBAVC1394_LIBS) $(GL_LIBS) $(GLU_LIBS)
mainTester_CPPFLAGS= $(AM_CPPFLAGS)  $(GST_CFLAGS) $(GTK_CFLAGS) $(GL_CFLAGS) $(GLU_CFLAGS)
