all:

%.png: %.msc
	mscgen -T png -i $< -o $@

%.svg: %.msc
	mscgen -T svg -i $< -o $@

%.svg: %.dot
	dot -Tsvg $< > $@.svg

%.png: %.dot
	dot -Tpng $< > $@.png

