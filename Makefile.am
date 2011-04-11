ACLOCAL_AMFLAGS = -I m4

# "foreign" means not GNU, and is common for avoiding boring messages
# about files organized differently from what GNU expects
AUTOMAKE_OPTIONS = foreign

SUBDIRS = src po data py doc utils man 

EXTRA_DIST = \
    RELEASE \
    NEWS \
    README

# pkg-config file:
DISTCLEANFILES = shared-video.pc
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = shared-video-@SHAREDVIDEO_API_VERSION@.pc

