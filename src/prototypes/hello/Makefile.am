include $(top_srcdir)/src/globals.mak

lib_LTLIBRARIES= libhello.la
libhello_la_SOURCES = hello.cpp

libhello_la_LDFLAGS = -version-info 1:1:1

# -fPIC required of archives that will be linked to a shared library on AMD64
#http://www.gentoo.org/proj/en/base/amd64/howtos/index.xml?part=1&chap=3
