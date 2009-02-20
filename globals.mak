src = $(top_srcdir)/src
INCLUDES = -I$(src)/include -I$(src) -I..
AM_CPPFLAGS = $(GLIB_CFLAGS)  
LIBUTIL = $(src)/util/libutil.a

uncrustify:
	-uncrustify -c $(top_srcdir)/utils/uncrustify.cfg -q --no-backup *.cpp
	-uncrustify -c $(top_srcdir)/utils/uncrustify.cfg -q --no-backup *.h

pylint: 
	for i in $(SUBDIRS); do \
        echo "make pylint in $$i..."; \
        (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) pylint); done
	@echo Checks with pylint if python files are buzzwords-compliant in $(PWD) 
	for pfile in *.py; do \
        pylint $$pfile; done 
