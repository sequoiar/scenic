SHELL=/bin/sh
APPLICATION=sipStateless 

include ../base.mak

PJBASE=$(BASE_ROOT)/pjsip

include $(PJBASE)/build.mak 
BINDIR=$(BASE)/bin
OBJDIR=$(BASE)/obj

OBJS=sipStateless.o sipSingleton.o 
MAINOBJS=$(OBJS) sipMain.o
TESTOBJS=$(OBJS) sipTestSuite.o


GST_CFLAGS=$(shell pkg-config --cflags gstreamer-0.10)
GST_LDFLAGS=$(shell pkg-config --libs gstreamer-0.10)

APP_CFLAGS+=-g -Wall $(GST_CFLAGS) 
CC		= $(APP_CC) 
CPP_CC  = g++
LDFLAGS = $(APP_LDFLAGS) $(GST_LDFLAGS)
LDLIBS = $(APP_LDLIBS) 
CFLAGS = $(APP_CFLAGS)
CPPFLAGS = $(CFLAGS)

%.o: %.c 
	$(CC) $(CPPFLAGS) -c $(<) -o $@

%.o: %.cpp
	$(CPP_CC) $(CPPFLAGS) -c $(<) -o $@

$(APPLICATION): $(MAINOBJS)
	$(CPP_CC) $(MAINOBJS) -o $(BINDIR)/$@ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) 

tests : $(TESTOBJS)
	$(CPP_CC) $(TESTOBJS) `pkg-config libcpptest --libs --cflags` -o $(BINDIR)/$@ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

all: $(APPLICATION)

tags: $(OBJS)
	ctags *.c *.cpp *.h ../gst/*.c ../gst/*.cpp ../gst/*.h

.PHONY: clean
clean:
	rm *.o
	rm $(BINDIR)/$(APPLICATION)

