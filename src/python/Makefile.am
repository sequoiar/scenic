include $(top_srcdir)/src/globals.mak

lib_LTLIBRARIES = libpyboostskel.la

libpyboostskel_la_SOURCES = exports.cpp
libpyboostskel_la_LDFLAGS = -version-info 1:1:1

# add your libs i.e. ../hello/libhello.la  
libpyboostskel_la_LIBADD =  $(src)/tcp/libtcp.a $(LIBUTIL) -l$(BOOST_PYTHON_LIB) $(GLIB_LIBS) 

libpyboostskel_la_CPPFLAGS = -I$(PYTHON_INCLUDE_DIR) $(AM_CPPFLAGS)

