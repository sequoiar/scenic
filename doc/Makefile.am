dist_pkgdata_DATA=footer.html header.html doxygen.css epydoc.css Doxyfile

all: 
	@for i in $(SUBDIRS); do \
        echo "make all in $$i..."; \
        (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) all); done

html: doxygen epydoc
	@for i in $(SUBDIRS); do \
        echo "make html in $$i..."; \
        (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) html); done
	
doxygen:
	@if [ -z `which doxygen` ]; then echo "No Doxygen!"; else doxygen; fi

epydoc:
	@if [ -z `which epydoc` ]; then echo "No epydoc!"; else epydoc --html --output pythondocs --url http://svn.sat.qc.ca/trac/scenic/ --name Scenic --verbose --simple-term --css epydoc.css ../py/scenic ../py/scripts/scenic ../py/scripts/midistream ../py/rtpmidi; fi

install: 
	@if test -e html; then \
    echo INSTALLING docs at $(DESTDIR)$(prefix); \
	mkdir -p $(DESTDIR)$(prefix)/share/doc/doxygen; \
	cp -rfv html $(DESTDIR)$(prefix)/share/doc/doxygen; \
	fi

uninstall:
	rm -rf $(DESTDIR)$(prefix)/share/doc/doxygen

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
