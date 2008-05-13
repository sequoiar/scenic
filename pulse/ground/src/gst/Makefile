SHELL=/bin/sh
APPLICATION=gst

include ../base.mak


BINDIR=$(BASE)/bin
OBJDIR=$(BASE)/obj
#SRCDIR=$(BASE)/src

#VPATH = $(SRCDIR)
#VPATH = $(OBJDIR)
OBJS=gst_main.o gstApp.o videoReceiver.o videoSender.o


GST_CFLAGS=$(shell pkg-config --cflags gstreamer-0.10)
GST_LDFLAGS=$(shell pkg-config --libs gstreamer-0.10)

APP_CFLAGS+=-g -Wall $(GST_CFLAGS) 
CC	= g++
CPP_CC  = g++
LDFLAGS = $(APP_LDFLAGS) $(GST_LDFLAGS)
LDLIBS = $(APP_LDLIBS) 
CFLAGS = $(APP_CFLAGS)
CPPFLAGS = $(CFLAGS)

%.o: %.c 
	$(CC) $(CPPFLAGS) -c $(<) -o $@

%.o: %.cpp %.h
	$(CPP_CC) $(CPPFLAGS) -c $(<) -o $@

$(APPLICATION): $(OBJS)
	$(CPP_CC) $(OBJS) -o $(BINDIR)/$@ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) 

all: $(APPLICATION)

tags:
	ctags *.c *.cpp *.h

.PHONY: clean
clean:
	rm $(OBJS)
	rm $(BINDIR)/$(APPLICATION)

