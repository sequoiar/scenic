src = $(top_srcdir)/src
inc = $(src)/include
INCLUDES = -I$(inc) -I$(src) -I..
AM_CPPFLAGS = $(GLIB_CFLAGS)  
LIBUTIL = $(top_builddir)/src/util/libutil.la

uncrustify:
	-uncrustify -c $(top_srcdir)/utils/uncrustify.cfg -q --no-backup *.cpp
	-uncrustify -c $(top_srcdir)/utils/uncrustify.cfg -q --no-backup *.h

pylint: $(addsuffix _pylint.txt,$(basename $(wildcard *.py)))
	for i in $(SUBDIRS); do \
    echo "Pass a pylint brush in $$i"; \
    (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) pylint); \
done 

%_pylint.txt: %.py
		pylint $^ > $@ 

#	for pfile in *.py; do \
    if [ $$pfile != "*.py" ]; then \
        pylint *.py; break; fi; \
done 
