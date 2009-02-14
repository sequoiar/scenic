#list of msc filenames with extension stripped
MSC_GLOB = $(basename $(wildcard *msc))
HTML_TARGETS = $(addsuffix _msc.svg,$(MSC_GLOB)) \
			   $(addsuffix _msc.png,$(MSC_GLOB)) \
			   $(addsuffix _dot.png,$(basename $(wildcard *dot))) \
			   $(addsuffix _dot.svg,$(basename $(wildcard *dot)))
all: 

html: $(HTML_TARGETS)

%_dot.png: %.dot
		dot -Tpng $< > $@

%_dot.svg: %.dot
		dot -Tsvg $< > $@

%_msc.png: %.msc
		mscgen -T png -i $< -o $@

%_msc.svg: %.msc 
		mscgen -T svg -i $< -o $@

install:

clean: 
	rm -f $(HTML_TARGETS)

moreclean:

.PHONY: html install clean moreclean
