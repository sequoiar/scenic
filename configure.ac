#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(SCENIC, 0.4.0)
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL

AM_INIT_AUTOMAKE([-Wno-portability])

# Check for C and C++ compilers
AC_PROG_CXX
AC_PROG_CC

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h sys/ioctl.h unistd.h linux/videodev2.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
PKG_PROG_PKG_CONFIG

AC_PROG_GCC_TRADITIONAL
AC_CHECK_FUNCS([inet_ntoa memset socket])

AC_MSG_CHECKING(whether to enable debugging)
AC_ARG_ENABLE([debug], [
  --enable-debug  debug symbols, build test programs, stricter compile ],, 
        enable_debug=yes)

AC_ARG_ENABLE([svn-revision], [ --enable-svn-revision ],,enable_svn_revision=no)
AM_CONDITIONAL([SVN],test "x${enable_svn_revision}" = "xyes")

AC_MSG_WARN([${ac_prefix}])
# Checks for library functions.

PKG_CHECK_MODULES(GLIB, [glib-2.0 gthread-2.0], have_glib=true, have_glib=false)
if test "x${have_glib}" = "xfalse" ; then
    AC_MSG_ERROR([Missing libglib2.0-dev])
fi

#Trying to set GTK+-2.0 using pkg-config
PKG_CHECK_MODULES(GTK, [gtk+-2.0], have_gtk=true, have_gtk=false)
if test "x${have_gtk}" = "xfalse" ; then
    AC_MSG_ERROR([Missing libgtk2.0-dev])
fi

#Trying to set GST_CFLAGS GST_LIBS using pkg-config
PKG_CHECK_MODULES(GST, gstreamer-0.10, have_gstreamer=true, have_gstreamer=false)
if test "x${have_gstreamer}" = "xfalse" ; then
    AC_MSG_ERROR([Missing libgstreamer0.10-dev GStreamer not found.])
fi

#Trying to set GSTAUDIO using pkg-config
PKG_CHECK_MODULES(GSTAUDIO, gstreamer-audio-0.10, have_gstreamer_audio=true, have_gstreamer_audio=false)
if test "x${have_gstreamer_audio}" = "xfalse" ; then
    AC_MSG_ERROR([missing pkg -  libgstreamer-plugins-base0.10-dev  ])
fi

#Trying to set GSTAPP using pkg-config
PKG_CHECK_MODULES(GSTAPP, gstreamer-app-0.10, have_gstreamer_app=true, have_gstreamer_app=false)
if test "x${have_gstreamer_app}" = "xfalse" ; then
    AC_MSG_ERROR([missing pkg -  libgstreamer-plugins-base0.10-dev  ])
fi

#Trying to set JACK using pkg-config
PKG_CHECK_MODULES(JACK, jack >= 0.99.10, have_jack=true, have_jack=false)
if test "x${have_jack}" = "xfalse" ; then
    AC_MSG_ERROR([missing pkg -  libjack0 and/or libjack-dev])
fi

#Trying to set LIBAVC1394 using pkg-config
PKG_CHECK_MODULES(LIBAVC1394, [libavc1394], have_libavc1394=true, have_libavc1394=false)
if test "x${have_libavc1394}" = "xfalse" ; then
	AC_MSG_ERROR([missing pkg - libavc1394-dev ])
fi

#Trying to set LIBRAW1394 using pkg-config
PKG_CHECK_MODULES(LIBRAW1394, [libraw1394], have_libraw1394=true, have_libraw1394=false)
if test "x${have_libraw1394}" = "xfalse" ; then
	AC_MSG_ERROR([missing pkg - libraw1394-dev ])
fi

#Trying to set LIBDC1394 using pkg-config
PKG_CHECK_MODULES(LIBDC1394, [libdc1394-2], have_libdc1394=true, have_libdc1394=false)
if test "x${have_libdc1394}" = "xfalse" ; then
	AC_MSG_ERROR([missing pkg - libdc1394-2])
fi

AC_SUBST(ALL_GST_LIBS, "$GST_LIBS $GSTAUDIO_LIBS $GSTAPP_LIBS")

dnl check for python, FIXME are we checking for python twice?
AM_PATH_PYTHON([2.5])


# BOOST m4 tests
AX_BOOST_BASE([1.35])
AX_BOOST_THREAD
AX_BOOST_SYSTEM
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_ASIO
AX_BOOST_DATE_TIME
AX_BOOST_FILESYSTEM

# GNU help2man creates man pages from --help output; in many cases, this
# is sufficient, and obviates the need to maintain man pages separately.
# However, this means invoking executables, which we generally cannot do
# when cross-compiling, so we test to avoid that (the variable
# "cross_compiling" is set by AC_PROG_CC).
if test $cross_compiling = no; then
  AM_MISSING_PROG(HELP2MAN, help2man)
else
  HELP2MAN=:
fi

if test "x${no_gl}" = "xyes" ; then
    AC_MSG_WARN([Missing OpenGL try some of mesa-common-dev libgl1-mesa-swx11-dev, xorg-driver-fglrx-dev nvidia-glx-new-dev.])
fi

AX_CHECK_GLU
if test "x${no_glu}" = "xyes" ; then
    AC_MSG_WARN([Missing GLU install libglu1-mesa-dev])
else
    AC_DEFINE(CONFIG_GL,[],[GL define from configure])
fi

if test "x${enable_debug}" = "xyes" ; then
    AC_DEFINE(CONFIG_DEBUG_LOCAL,[],[Debug define from configure])
    CXXFLAGS="-g -O0 -rdynamic -Wextra -Weffc++ -pedantic -Wfatal-errors -Werror -Wno-variadic-macros -Wno-long-long -Wall -Wshadow"
fi

CXXFLAGS+=" -fPIC "

AM_CONDITIONAL([PROJ_DEBUG],test "x${enable_debug}" = "xyes")

# FIXME
SCENIC_LOCALE_DIR=po
AC_SUBST(SCENIC_LOCALE_DIR)

AS_AC_EXPAND(LIBDIR, $libdir)
AC_MSG_NOTICE(Storing library files in $LIBDIR)

PYTHON_LIB_PATH=$LIBDIR/scenic/py
AC_SUBST(PYTHON_LIB_PATH)

AC_CONFIG_FILES([py/scripts/scenic], [chmod +x py/scripts/scenic])
AC_CONFIG_FILES([py/scripts/midistream], [chmod +x py/scripts/midistream])

AC_CONFIG_FILES([
        Makefile
        doc/Makefile
        doc/network/Makefile
        doc/figures/Makefile
        man/Makefile
        utils/Makefile
        utils/dc-ctl/Makefile
        src/Makefile
        py/Makefile
        py/doc/Makefile
        py/rtpmidi/Makefile
        py/rtpmidi/engines/Makefile
        py/rtpmidi/engines/midi/Makefile
        py/rtpmidi/protocols/Makefile
        py/rtpmidi/protocols/rtp/Makefile
        py/scenic/Makefile
        py/scenic/configure.py
        py/scenic/devices/Makefile
        py/scenic/data/Makefile
        po/Makefile
        po/en_US/Makefile
        po/en_US/LC_MESSAGES/Makefile
        po/fr_CA/Makefile
        po/fr_CA/LC_MESSAGES/Makefile
        py/scenic/data/themes/Makefile
        py/scenic/data/themes/Darklooks/Makefile
        py/scenic/test/Makefile
        py/scripts/Makefile
        src/include/Makefile
        src/util/Makefile
        src/gutil/Makefile
        src/shared_video/Makefile
        src/tcp/Makefile
        src/gst/Makefile
        src/firereset/Makefile
        src/main/Makefile
        src/main/test/Makefile
        ])

AC_OUTPUT
