src = $(top_srcdir)/src
INCLUDES = -I$(src)/include -I$(src) -I..
AM_CPPFLAGS = $(GLIB_CFLAGS)  
LIBUTIL = $(src)/util/libutil.a

uncrustify:
	-uncrustify -c $(top_srcdir)/utils/uncrustify.cfg -q --no-backup *.cpp
	-uncrustify -c $(top_srcdir)/utils/uncrustify.cfg -q --no-backup *.h

pylint: 
	for i in $(SUBDIRS); do \
    echo "Pass a pylint brush in $$i"; \
    (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) pylint); \
done 
	for pfile in *.py; do \
    if [ $$pfile != "*.py" ]; then \
    pylint $$pfile; fi; \
done 
