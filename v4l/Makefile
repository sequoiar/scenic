VERSION=0.9

#CFLAGS+=-O2 -Wall -DVERSION=\"$(VERSION)\" $(DEBUG)
CFLAGS+=-g -Wall -DVERSION=\"$(VERSION)\" $(DEBUG)
LDFLAGS+=

OBJS=dov4l.o

all: dov4l

dov4l: $(OBJS)
	$(CC) -Wall -W $(OBJS) $(LDFLAGS) -o dov4l

install: dov4l
	cp dov4l /usr/bin
	install -D dov4l.1 /usr/man/man1/dov4l.1
	rm -f /usr/man/man1/dov4l.1.gz
	gzip -9 /usr/man/man1/dov4l.1

uninstall: clean
	rm -f /usr/bin/dov4l
	rm -f /usr/man/man1/dov4l.1.gz

clean:
	rm -f $(OBJS) dov4l core

package: clean
	# source package
	rm -rf dov4l-$(VERSION)*
	mkdir dov4l-$(VERSION)
	cp *.c dov4l.1 Makefile license.txt dov4l-$(VERSION)
	tar czf dov4l-$(VERSION).tgz dov4l-$(VERSION)
	rm -rf dov4l-$(VERSION)
