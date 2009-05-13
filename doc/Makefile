SUBDIRS = notes test figures figures/test

all: 
	@for i in $(SUBDIRS); do \
        echo "make all in $$i..."; \
        (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) all); done

html: doxygen
	@for i in $(SUBDIRS); do \
        echo "make html in $$i..."; \
        (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) html); done
	
doxygen:
	@doxygen

install:
	@if test -e html; then \
	mkdir -p $(DESTDIR)$(prefix)/share/doc/doxygen \
	cp -rfv html $(DESTDIR)$(prefix)/share/doc/doxygen; \
	fi

uninstall:
	rm -rf /share/doc/doxygen

check:
	@echo tests for docs?

clean:
	rm -rf html latex xml rtf def
	@for i in $(SUBDIRS); do \
        echo "Clearing in $$i..."; \
        (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) clean); done

moreclean:
	@for i in $(SUBDIRS); do \
        echo "Clearing in $$i..."; \
        (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) moreclean); done

distclean: clean

distdir:

.PHONY: all html doxygen install check clean moreclean distclean
