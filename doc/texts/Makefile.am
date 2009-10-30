#list of rst filenames with extension stripped
RST_GLOB = $(basename $(wildcard *rst))
HTML_TARGETS = $(addsuffix _rst.html,$(RST_GLOB))
all: 

html: $(HTML_TARGETS)

%_rst.html: %.rst
		rst2html $< > $@

install:

clean: 
	rm -f $(HTML_TARGETS)

moreclean:

.PHONY: html install clean moreclean
