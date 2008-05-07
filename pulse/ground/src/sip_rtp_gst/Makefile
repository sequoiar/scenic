PJBASE=~/devel/pjsip

include $(PJBASE)/build.mak

BASE=~/devel/miville/trunk/pulse/ground/
BINDIR=${BASE}bin/

CC		= $(APP_CC)
LDFLAGS = $(APP_LDFLAGS)
LDLIBS = $(APP_LDLIBS)
CFLAGS = $(APP_CFLAGS)
CPPFLAGS = ${CFLAGS}

all: siprtp

siprtp: siprtp.c
	$(CC) -o ${BINDIR}$@ $< $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)
