# Makefile for the sip module

proj_src = $(top_srcdir)/src
BINdir = /usr/bin

# -lpjsua -lpjsip -lpjmedia -lpjsip-simple -lpjsip-ua -lpjmedia-codec -lpjlib-util -lpj
# PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjsip-ua -lpjmedia -lpjmedia-codec
PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjmedia 

BIN_PROGRAMS = session

session_SOURCES = \
		     session.c

LDADD = $(PJSIP_LIBS) -lpthread 
AM_CFLAGS = $(PJSIP_CFLAGS)

noinst_HEADERS = \
		 session.h 
