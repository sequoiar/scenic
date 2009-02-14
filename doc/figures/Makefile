#list of msc filenames with extension stripped
MSC_GLOB = $(basename $(wildcard *msc))

all: 

html: $(addsuffix .svg,$(MSC_GLOB)) $(addsuffix .png,$(MSC_GLOB))


%.png: %.msc
	mscgen -T png -i $< -o $@

%.svg: %.msc
	mscgen -T svg -i $< -o $@

install:

clean: $(addsuffix .png,$(MSC_GLOB)) $(addsuffix .svg,$(MSC_GLOB))
	rm -f $^

moreclean:

.PHONY: html install clean moreclean
