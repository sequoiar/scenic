#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(MILHOUSE, 0.3.6)
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL

AM_INIT_AUTOMAKE([-Wno-portability])

#AM_CONFIG_HEADER(config.h)
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

AC_ARG_ENABLE([millisecwait], [ --enable-millisecwait=(millisecs) thread sleep time],,enable_millisecwait=10)
AC_DEFINE_UNQUOTED(MILLISEC_WAIT,[${enable_millisecwait}], wait time)

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

AX_PYTHON
dnl check for python
AS_PATH_PYTHON(2.3)

# BOOST m4 tests
AX_BOOST_BASE([1.35])
AX_BOOST_THREAD
AX_BOOST_SYSTEM
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_ASIO
AX_BOOST_DATE_TIME
AX_BOOST_FILESYSTEM

AX_CHECK_GL
AX_CHECK_GLU

AC_PROG_HELP2MAN

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

if test "x${have_libavc1394}" = "xfalse" -o "x${have_libavc1394}" = "xfalse" ; then
AC_MSG_NOTICE([undef CONFIG_IEEE1394])
else
AC_DEFINE(CONFIG_IEEE1394,[],[IEEE1934 define from configure])
AC_MSG_NOTICE([define CONFIG_IEEE1394])
fi

AC_OUTPUT([Makefile
        doc/Makefile
        doc/notes/Makefile
        doc/test/Makefile
        doc/network/Makefile
        doc/texts/Makefile
        doc/figures/Makefile
        doc/figures/test/Makefile
        doc/sequences/Makefile
        utils/Makefile
        utils/dc-ctl/Makefile
        src/Makefile
        py/Makefile
        py/doc/Makefile
        py/scenic/Makefile
        py/scenic/configure.py
        py/scenic/devices/Makefile
        py/scenic/data/Makefile
        py/scripts/Makefile
        py/scenic/test/Makefile
        src/include/Makefile
        src/util/Makefile
        src/gutil/Makefile
        src/shared_video/Makefile
        src/tcp/Makefile
        src/gst/Makefile
        src/gst/engine/Makefile
        src/devices/Makefile
        src/main/Makefile])

AC_OUTPUT([
        src/gst/engine/test/Makefile
        ])


AC_OUTPUT
echo

    if test "x${have_libavc1394}" = "xfalse" -o "x${have_libavc1394}" = "xfalse" ; then
AC_MSG_NOTICE([Not building 1394 support])
    fi

    if test "x${no_glu}" = "xyes" ; then
AC_MSG_NOTICE([Not building OpenGL support])
    fi

