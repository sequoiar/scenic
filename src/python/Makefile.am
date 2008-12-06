include $(top_srcdir)/src/globals.mak

lib_LTLIBRARIES = libmsgthreads.la

libmsgthreads_la_SOURCES = exports.cpp
libmsgthreads_la_LDFLAGS = -version-info 1:1:1

# add your libs i.e. ../hello/libhello.la  
libmsgthreads_la_LIBADD =  $(src)/tcp/libtcp.a $(LIBUTIL) -l$(BOOST_PYTHON_LIB) $(GLIB_LIBS) 

libmsgthreads_la_CPPFLAGS = -I$(PYTHON_INCLUDE_DIR) $(AM_CPPFLAGS)

