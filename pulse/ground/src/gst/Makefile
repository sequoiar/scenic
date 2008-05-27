SHELL=/bin/sh
APPLICATION=gst

include ../base.mak

PJBASE=$(BASE_ROOT)/pjsip

include $(PJBASE)/build.mak 


BINDIR=$(BASE)/bin
OBJDIR=$(BASE)/obj

OBJS=videoReceiver.o videoSender.o videoBase.o audioBase.o \
	 audioSender.o
APPOBJS=$(OBJS) gst_main.o gstApp.o 
TESTOBJS=$(OBJS) gstTestSuite.o 

GST_CFLAGS=$(shell pkg-config --cflags gstreamer-0.10)
GST_LDFLAGS=$(shell pkg-config --libs gstreamer-0.10)

APP_CFLAGS+=-g -Wall $(GST_CFLAGS) 
CC	= g++
CPP_CC  = g++
LDFLAGS = $(APP_LDFLAGS) $(GST_LDFLAGS)
LDLIBS = $(APP_LDLIBS) 
CFLAGS = $(APP_CFLAGS)
CPPFLAGS = $(CFLAGS)

%.o: %.c ../sip/%.o
	$(CC) $(CPPFLAGS) -c $(<) -o $@

%.o: %.cpp %.h ../sip/%.o 
	$(CPP_CC) $(CPPFLAGS) -c $(<) -o $@

$(APPLICATION): $(APPOBJS)
	$(CPP_CC) $(APPOBJS) ../sip/sipSingleton.o ../sip/sipStateless.o -o $(BINDIR)/$@ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) 

tests : $(TESTOBJS)
	$(CPP_CC) $(TESTOBJS) `pkg-config libcpptest --libs --cflags` -o $(BINDIR)/gstTester $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)
					 
all: $(APPLICATION)

tags: $(OBJS)
	ctags *.c *.cpp *.h ../sip/*.c ../sip/*.cpp ../sip/*.h

.PHONY: clean
clean:
	rm $(OBJS)
	rm $(BINDIR)/$(APPLICATION)

