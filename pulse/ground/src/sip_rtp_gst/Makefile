APPLICATION=siprtp

PJBASE=~/devel/pjsip

include $(PJBASE)/build.mak

BASE=~/devel/miville/trunk/pulse/ground/
BINDIR=$(BASE)bin/

#OBJS=main_entry.o siprtp.o

GST_FLAGS=`pkg-config --cflags --libs gstreamer-0.10`

APP_CFLAGS+=-g -Wall $(GST_FLAGS)
SOURCES=main_entry.c siprtp.c gstbuff_ready_cb.c
OBJS:=$(addsuffix .o, $(basename $(SOURCES)))
CC		= $(APP_CC)
LDFLAGS = $(APP_LDFLAGS)
LDLIBS = $(APP_LDLIBS)
CFLAGS = $(APP_CFLAGS)
CPPFLAGS = $(CFLAGS)

all: $(APPLICATION)

$(APPLICATION) : $(OBJS)
	$(CC) -o $(BINDIR)$@ $< $(CPPFLAGS) $(LDFLAGS) $^ $(LDLIBS) 


.PHONY: clean
clean:
	rm $(BINDIR)$(APPLICATION)
