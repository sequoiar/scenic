SHELL=/bin/sh
APPLICATION=gst

include ../base.mak

PJBASE=$(BASE_ROOT)/pjsip

include $(PJBASE)/build.mak 


BINDIR=$(BASE)/bin
OBJDIR=$(BASE)/obj

OBJS=videoReceiver.o videoSender.o mediaBase.o audioSender.o audioReceiver.o
APPOBJS=$(OBJS) gst_main.o gstApp.o 
TESTOBJS=$(OBJS) gstTestSuite.o 
PYTHONOBJS=$(OBJS) py$(APPLICATION)Module.o

GST_CFLAGS=-pthread -I/usr/local/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libxml2 
GST_LDFLAGS=-pthread -L/usr/local/lib -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -ldl -lgthread-2.0 -lrt -lxml2 -lglib-2.0  

CPPTEST_LDFLAGS=-lcpptest

APP_CFLAGS+=-g -Wall $(GST_CFLAGS) $(JACK_CFLAGS)
CC	= g++
CPP_CC  = g++
LDFLAGS = $(APP_LDFLAGS) $(GST_LDFLAGS) $(JACK_LDFLAGS)
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
	$(CPP_CC) $(TESTOBJS) $(CPPTEST_LDFLAGS) -o $(BINDIR)/$(APPICATION)Tester $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)
					 
all: $(APPLICATION)

tags: 
	ctags *.c *.cpp *.h ../sip/*.c ../sip/*.cpp ../sip/*.h

.PHONY: clean
clean:
	rm *.o; rm $(BINDIR)/$(APPLICATION); rm $(BINDIR)/$(APPLICATION)Tester

