SUBDIRS = docbook 
dist_pkgdata_DATA=footer.html header.html doxygen.css epydoc.css Doxyfile

html-local: doxygen epydoc
    
doxygen:
	@if [ -z `which doxygen` ]; then echo "No Doxygen!"; else doxygen; fi

epydoc:
	@if [ -z `which epydoc` ]; \
		then echo "No epydoc!"; \
		else \
			epydoc  --html \
					--output pythondocs \
					--url http://svn.sat.qc.ca/trac/scenic/ \
					--name Scenic \
					--verbose \
					--simple-term \
					--css epydoc.css ../py/scenic ../py/scripts/scenic ../py/scripts/midistream ../py/rtpmidi; \
	 fi

clean-local:
	rm -rf html pythondocs
