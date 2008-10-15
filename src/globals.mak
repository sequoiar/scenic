src = $(top_srcdir)/src
INCLUDES = -I$(src)/include -I$(src) -I..
AM_CPPFLAGS = $(GLIB_CFLAGS) 
LIBUTIL = $(src)/util/libUtil.a

uncrustify:
	-uncrustify -c $(top_srcdir)/utils/uncrustify.cfg -q --no-backup *.cpp
	-uncrustify -c $(top_srcdir)/utils/uncrustify.cfg -q --no-backup *.h

