lib_LTLIBRARIES = libshared_video-@SHAREDVIDEO_API_VERSION@.la
 # public header file 
libshared_video_@SHAREDVIDEO_API_VERSION@_la_includedir = $(includedir)/shared-video-@SHAREDVIDEO_API_VERSION@

libshared_video_@SHAREDVIDEO_API_VERSION@_la_include_HEADERS = \
    $(top_srcdir)/src/include/sharedVideoBuffer.h \
    $(top_srcdir)/src/include/videoSize.h

libshared_video_@SHAREDVIDEO_API_VERSION@_la_SOURCES = \
    sharedVideoBuffer.cpp \
    $(top_srcdir)/src/include/sharedVideoBuffer.h

libshared_video_@SHAREDVIDEO_API_VERSION@_la_CPPFLAGS = -I$(top_srcdir)/src/include

libshared_video_@SHAREDVIDEO_API_VERSION@_la_LDFLAGS = -export-dynamic -version-info $(SHAREDVIDEO_SO_VERSION)

