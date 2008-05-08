APPLICATION=siprtp

PJBASE=~/devel/pjsip

include $(PJBASE)/build.mak

BASE=~/devel/miville/trunk/pulse/ground/
BINDIR=$(BASE)bin/

#OBJS=main_entry.o siprtp.o


#FIXME: there must be a better way of determing these
GST_FLAGS=-I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libxml2 -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -ldl -lgthread-2.0 -lrt -lxml2 -lglib-2.0

APP_CFLAGS+=-g -Wall $(GST_FLAGS)
SOURCES=main_entry.c siprtp.c gstbuff_ready_cb.c
CC		= $(APP_CC)
LDFLAGS = $(APP_LDFLAGS)
LDLIBS = $(APP_LDLIBS)
CFLAGS = $(APP_CFLAGS)
CPPFLAGS = $(CFLAGS)

all: $(APPLICATION)

$(APPLICATION) : 
	$(CC) -o $(BINDIR)$@ $< $(CPPFLAGS) $(LDFLAGS) $(SOURCES) $(LDLIBS) 

