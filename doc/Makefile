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
	@for i in $(SUBDIRS); do \
        echo "Installing in $$i..."; \
        (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) install); done

clean:
	rm -rf html latex xml rtf
	@for i in $(SUBDIRS); do \
        echo "Clearing in $$i..."; \
        (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) clean); done

moreclean:
	@for i in $(SUBDIRS); do \
        echo "Clearing in $$i..."; \
        (cd $$i; $(MAKE) $(MFLAGS) $(MYMAKEFLAGS) moreclean); done

.PHONY: html doxygen install clean moreclean
