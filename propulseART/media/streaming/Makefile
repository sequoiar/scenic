PJBASE=/home/${USER}/pjsip

include $(PJBASE)/build.mak

CC		= $(APP_CC)
LDFLAGS = $(APP_LDFLAGS)
LDLIBS = $(APP_LDLIBS)
CFLAGS = $(APP_CFLAGS)
CPPFLAGS = ${CFLAGS}

all: siprtp

siprtp: siprtp.c
	$(CC) -o $@ $< $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)
