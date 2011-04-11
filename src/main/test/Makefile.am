TESTS = check_pipeline 
check_PROGRAMS = check_pipeline 

check_pipeline_SOURCES = check_pipeline.cpp $(top_srcdir)/src/gutil/gutil.h $(top_srcdir)/src/gutil/gutil.cpp \
                    $(top_srcdir)/src/gst/pipeline.h $(top_srcdir)/src/gst/pipeline.cpp
check_pipeline_LDADD = $(top_builddir)/src/gutil/libgutil.la $(top_builddir)/src/gst/libgst.la $(top_builddir)/src/util/libutil.la $(GST_LIBS) $(GSTAUDIO_LIBS) $(LIBRAW1394_LIBS) $(LIBAVC1394_LIBS) $(LIBDC1394_LIBS) $(GTK_LIBS)
INCLUDES = -I$(top_srcdir)/src/include -I$(top_srcdir)/src
AM_CXXFLAGS = $(GST_CFLAGS) $(GTK_CFLAGS)
