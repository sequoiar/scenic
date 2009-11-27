#list of msc filenames with extension stripped
MSC_GLOB = $(basename $(wildcard *msc))
HTML_TARGETS = $(addsuffix _msc.svg,$(MSC_GLOB)) \
			   $(addsuffix _dot.svg,$(basename $(wildcard *dot)))
all: 

html: $(HTML_TARGETS)

%_dot.svg: %.dot
		dot -Tsvg $< > $@

%_msc.svg: %.msc 
		mscgen -T svg -i $< -o $@

install:

clean: 
	rm -f $(HTML_TARGETS)

moreclean:

.PHONY: html install clean moreclean
