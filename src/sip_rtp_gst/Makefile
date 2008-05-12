SHELL=/bin/sh
APPLICATION=siprtp

include ../base.mak

PJBASE=$(BASE_ROOT)/pjsip

include $(PJBASE)/build.mak

BINDIR=$(BASE)/bin
OBJDIR=$(BASE)/obj
#SRCDIR=$(BASE)/src

VPATH = $(SRCDIR)
VPATH = $(OBJDIR)
SOURCES=sip_rtp_gst_main.cpp siprtp.c gstbuff_ready_cb.c
OBJS=sip_rtp_gst_main.o siprtp.o gstbuff_ready_cb.o


GST_CFLAGS=$(shell pkg-config --cflags gstreamer-0.10)
GST_LDFLAGS=$(shell pkg-config --libs gstreamer-0.10)

APP_CFLAGS+=-g -Wall $(GST_CFLAGS) 
CC		= $(APP_CC)
LDFLAGS = $(APP_LDFLAGS) $(GST_LDFLAGS)
LDLIBS = $(APP_LDLIBS) 
CFLAGS = $(APP_CFLAGS)
CPPFLAGS = $(CFLAGS)

%.o: %.c %.cpp
	$(CC) $(CPPFLAGS) -c $(<) -o $(OBJDIR)/$@

$(APPLICATION): $(OBJS)
	$(CC) $(OBJDIR)/*.o -o $(BINDIR)/$@ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) 

all: $(APPLICATION)

tags:
	ctags *.c *.h

.PHONY: clean
clean:
	rm $(OBJDIR)/*.o
	rm $(BINDIR)/$(APPLICATION)

