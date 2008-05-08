APPLICATION=siprtp

include ../base.mak

PJBASE=$(BASE_ROOT)/pjsip

include $(PJBASE)/build.mak

BINDIR=$(BASE)bin
OBJDIR=$(BASE)obj
vpath %.o $(OBJDIR)

OBJS=main_entry.o siprtp.o gstbuff_ready_cb.o

#OBJS=main_entry.o siprtp.o

GST_FLAGS=$(shell pkg-config --cflags --libs gstreamer-0.10)

APP_CFLAGS+=-g -Wall $(GST_FLAGS)
CC		= $(APP_CC)
LDFLAGS = $(APP_LDFLAGS)
LDLIBS = $(APP_LDLIBS)
CFLAGS = $(APP_CFLAGS)
CPPFLAGS = $(CFLAGS)

%.o : %.c
	$(CC) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) -c $(<) -o $(OBJDIR)/$@ 

$(APPLICATION) : $(OBJS)
	$(CC) -o $(BINDIR)/$@ $(OBJDIR)/*.o $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) 

all: $(APPLICATION)

.PHONY: clean
clean:
	rm $(OBJDIR)/*.o
	rm $(BINDIR)/$(APPLICATION)
