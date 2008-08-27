# Gobject and glib stuff is common to both gstreamer and gtk, so repeated flags are omitted in GTK flags/libs variables

GST_CFLAGS=-pthread -I/usr/local/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libxml2  
GST_LIBS=-L/usr/local/lib -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -ldl -lgthread-2.0 -lrt -lxml2 -lglib-2.0 ~/gst-plugins-base/gst-libs/gst/interfaces/.libs/libgstinterfaces-0.10.so
GTK_CFLAGS=-I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/pixman-1
GTK_LIBS=-lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangocairo-1.0 -lpango-1.0 -lcairo -ldl 

CFLAGS=$(GST_CFLAGS) $(GTK_CFLAGS)
LIBS=$(GST_LIBS) $(GTK_LIBS)

all: test

test: test.c
	gcc $(CFLAGS) $(LIBS) $< -o $@
