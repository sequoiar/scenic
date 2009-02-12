all: index.html
	

.html.svg:
	@echo '<HTML><HEAD><meta http-equiv="refresh" content="2;url=figures.html"> </HEAD><BODY></BODY></HTML>' >> index.html

.png.msc: 
	mscgen -T png -i $< -o $@

.svg.msc:
	mscgen -T svg -i $< -o $@

clean:
	rm -f *.png
	rm -f *.svg
	rm -f index.html

