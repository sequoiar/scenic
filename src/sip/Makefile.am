# Makefile for the sip module

proj_src = $(top_srcdir)/src
BINdir = /usr/bin

# -lpjsua -lpjsip -lpjmedia -lpjsip-simple -lpjsip-ua -lpjmedia-codec -lpjlib-util -lpj
# PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjsip-ua -lpjmedia -lpjmedia-codec
PJSIP_LIBS = -lpjsip -lpjsip-ua  -lpjlib-util -lpj -lpjsua -lpjmedia 

#SUBDIRS=test

noinst_LTLIBRARIES = libsip.la

libsip_la_SOURCES = \
		     sdpcodec.cpp \
		     sdp.cpp \
		     useragent.cpp \
		     sipsession.cpp 

libsip_la_LDFLAGS = $(PJSIP_LIBS) -lpthread 
AM_CXXFLAGS = -I$(top_srcdir)/include `pkg-config --cflags libpj` 

noinst_HEADERS = \
		 sdpcodec.h \
		 sdp.h \
		 useragent.h \
		 session.h \
		 sipsession.h 
