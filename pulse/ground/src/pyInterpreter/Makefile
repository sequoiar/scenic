SHELL=/bin/sh
APPLICATION=python_main

include ../base.mak

#PJBASE=$(BASE_ROOT)/pjsip

#include $(PJBASE)/build.mak 


BINDIR=$(BASE)/bin
OBJDIR=$(BASE)/obj

#OBJS=videoReceiver.o videoSender.o mediaBase.o audioSender.o audioReceiver.o
OBJS=pyInterpreter.o
APPOBJS=$(OBJS) pyInterpreter_main.o
TESTOBJS=$(OBJS) gstTestSuite.o 
PYTHONOBJS=$(OBJS) py$(APPLICATION)Module.o

GST_CFLAGS=-pthread -I/usr/local/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libxml2 
GST_LDFLAGS=-pthread -L/usr/local/lib -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -ldl -lgthread-2.0 -lrt -lxml2 -lglib-2.0  

PY_CFLAGS=-I/usr/include/python2.5 -I/usr/include/boost/python 
PY_LDLIBS= /usr/lib/libboost_python-gcc42-1_34_1.a  /usr/lib/libpython2.5.so.1.0 

JACK_LDFLAGS=-ljack -lrt

CPPTEST_LDFLAGS=-lcpptest

APP_CFLAGS+=-g -Wall $(GST_CFLAGS) $(JACK_CFLAGS)
CC	= g++
CPP_CC  = g++
LDFLAGS = $(APP_LDFLAGS) $(GST_LDFLAGS) $(JACK_LDFLAGS)
LDLIBS = $(APP_LDLIBS) $(PY_LDLIBS)
CFLAGS = $(APP_CFLAGS) $(PY_CFLAGS)
CPPFLAGS = $(CFLAGS)

%.o: %.c ../sip/%.o
	$(CC) $(CPPFLAGS) -c $(<) -o $@

%.o: %.cpp %.h ../sip/%.o 
	$(CPP_CC) $(CPPFLAGS) -c $(<) -o $@

$(APPLICATION): $(APPOBJS)
	$(CPP_CC) $(APPOBJS) ../gst/mediaBase.o ../gst/videoSender.o ../hello/hello.o -o $(BINDIR)/$@ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) 

tests : $(TESTOBJS)
	$(CPP_CC) $(TESTOBJS) $(CPPTEST_LDFLAGS) -o $(BINDIR)/gstTester $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)
					 
all: $(APPLICATION)

tags: 
	ctags *.c *.cpp *.h ../sip/*.c ../sip/*.cpp ../sip/*.h

.PHONY: clean
clean:
	rm *.o
	rm $(BINDIR)/$(APPLICATION)
