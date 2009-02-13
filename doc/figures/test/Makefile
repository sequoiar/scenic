all: $(addsuffix .svg,$(basename $(wildcard *msc))) $(addsuffix .png,$(basename $(wildcard *msc)))


%.png: %.msc
	mscgen -T png -i $< -o $@

%.svg: %.msc
	mscgen -T svg -i $< -o $@


