SHELL=/bin/sh
APPLICATION=sip

include ../base.mak

PJBASE=$(BASE_ROOT)/pjsip

include $(PJBASE)/build.mak 
BINDIR=$(BASE)/bin
OBJDIR=$(BASE)/obj

EXT_OBJS=../sdp/sdp.o ../gst/mediaBase.o ../gst/videoSender.o ../gst/videoReceiver.o ../gst/audioSender.o ../gst/audioReceiver.o
OBJS=$(EXT_OBJS) sipStateless.o sipSingleton.o 
APPOBJS=$(OBJS) sipMain.o
TESTOBJS=$(OBJS) sipTestSuite.o



GST_CFLAGS=-pthread -I/usr/local/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libxml2 

GST_LDFLAGS=-pthread -L/usr/local/lib -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -ldl -lgthread-2.0 -lrt -lxml2 -lglib-2.0  

CPPTEST_LDFLAGS=-lcpptest

APP_CFLAGS+=-g -Wall $(GST_CFLAGS)  -I../
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

$(APPLICATION): $(APPOBJS)
	$(CPP_CC) $(APPOBJS) -o $(BINDIR)/$@ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) 

tests : $(TESTOBJS)
	$(CPP_CC) $(TESTOBJS) $(CPPTEST_LDFLAGS) -o $(BINDIR)/$(APPLICATION)Tester $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

all: $(APPLICATION)

tags: $(OBJS)
	ctags *.c *.cpp *.h ../gst/*.c ../gst/*.cpp ../gst/*.h

.PHONY: clean
clean:
	rm *.o
	rm $(BINDIR)/$(APPLICATION)

