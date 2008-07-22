include $(top_srcdir)/src/globals.mak

lib_LTLIBRARIES = libpyhello.la

libpyhello_la_SOURCES = pythonImport.cpp
libpyhello_la_LDFLAGS = -version-info 1:1:1

libpyhello_la_LIBADD = ../hello/libhello.la  -l$(BOOST_PYTHON_LIB)  

libpyhello_la_CXXFLAGS = -I$(PYTHON_INCLUDE_DIR)

