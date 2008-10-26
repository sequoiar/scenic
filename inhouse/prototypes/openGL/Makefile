all: square

CFLAGS=-Wall -Werror `pkg-config --cflags gtk+-2.0` `pkg-config --cflags gtkglext-1.0` `pkg-config --cflags gtkglext-x11-1.0`


LIBS=`pkg-config --libs gtk+-2.0` `pkg-config --libs gtkglext-1.0` `pkg-config --libs gtkglext-x11-1.0`


square: square.c
	gcc $(CFLAGS) $(LIBS) $< -o $@
