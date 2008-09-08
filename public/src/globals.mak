src = $(top_srcdir)/src
INCLUDES = -I$(src)/include -I$(src) -I..
AM_CPPFLAGS = $(GLIB_CFLAGS) 

all-local:
	-uncrustify -c $(top_srcdir)/utils/uncrustify.cfg --mtime -q --no-backup *.cpp
	-uncrustify -c $(top_srcdir)/utils/uncrustify.cfg --mtime -q --no-backup *.h
