#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(SCENIC, 0.5)  # should always be .1 greater than last tag, unless this is a tag
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_SRCDIR([src/main/milhouse.cpp])
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
    AC_MSG_ERROR([missing package:  libgstreamer-plugins-base0.10-dev  ])
fi

#Trying to set GSTAPP using pkg-config
PKG_CHECK_MODULES(GSTAPP, gstreamer-app-0.10, have_gstreamer_app=true, have_gstreamer_app=false)
if test "x${have_gstreamer_app}" = "xfalse" ; then
    AC_MSG_ERROR([missing package:  libgstreamer-plugins-base0.10-dev  ])
fi

#Trying to set JACK using pkg-config
PKG_CHECK_MODULES(JACK, jack >= 0.99.10, have_jack=true, have_jack=false)
if test "x${have_jack}" = "xfalse" ; then
    AC_MSG_ERROR([missing package:  libjack0 and/or libjack-dev])
fi

#Trying to set LIBAVC1394 using pkg-config
PKG_CHECK_MODULES(LIBAVC1394, [libavc1394], have_libavc1394=true, have_libavc1394=false)
if test "x${have_libavc1394}" = "xfalse" ; then
	AC_MSG_ERROR([missing package: libavc1394-dev ])
fi

#Trying to set LIBRAW1394 using pkg-config
PKG_CHECK_MODULES(LIBRAW1394, [libraw1394], have_libraw1394=true, have_libraw1394=false)
if test "x${have_libraw1394}" = "xfalse" ; then
	AC_MSG_ERROR([missing package: libraw1394-dev ])
fi

# FIXME: make this optional
AC_DEFINE(CONFIG_IEEE1394,[],[Everything in raw1394 is defined from configure])

#Trying to set LIBDC1394 using pkg-config
PKG_CHECK_MODULES(LIBDC1394, [libdc1394-2], have_libdc1394=true, have_libdc1394=false)
if test "x${have_libdc1394}" = "xfalse" ; then
	AC_MSG_ERROR([missing package: libdc1394-2])
fi

AC_SUBST(ALL_GST_LIBS, "$GST_LIBS $GSTAUDIO_LIBS $GSTAPP_LIBS")

dnl check for python
AM_PATH_PYTHON([2.5])

dnl BOOST m4 tests
AX_BOOST_BASE([1.35], [], [echo "libboost-1.35 or greater required"; exit -1])
AX_BOOST_THREAD
if test "x${ax_cv_boost_thread}" = "xno" ; then
    AC_MSG_ERROR([missing package: libboost-thread-dev])
fi
AX_BOOST_SYSTEM
if test "x${ax_cv_boost_system}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-system-dev])
fi
AX_BOOST_PROGRAM_OPTIONS
if test "x${ax_cv_boost_program_options}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-program-options-dev])
fi
AX_BOOST_ASIO
if test "x${ax_cv_boost_asio}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-asio-dev])
fi
AX_BOOST_DATE_TIME
if test "x${ax_cv_boost_date_time}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-date-time-dev])
fi
AX_BOOST_FILESYSTEM
if test "x${ax_cv_boost_filesystem}" = "xno" ; then
	AC_MSG_ERROR([missing package: libboost-filesystem-dev])
fi

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

AX_CHECK_GL
if test "x${no_gl}" = "xyes" ; then
    AC_MSG_WARN([Missing OpenGL try some of mesa-common-dev libgl1-mesa-swx11-dev, xorg-driver-fglrx-dev nvidia-glx-new-dev.])
fi

AX_CHECK_GLU
if test "x${no_glu}" = "xyes" ; then
    AC_MSG_WARN([Missing GLU install libglu1-mesa-dev])
else
    AC_DEFINE(CONFIG_GL,[],[GL define from configure])
fi

# Check for python modules, if second arg is non-empty,
# configure exits with an error (NOTE: it will not print this argument!)

AX_PYTHON_MODULE([pypm], [])
AS_IF([test "x${HAVE_PYMOD_PYPM}" = "xno"], 
        [AC_MSG_ERROR([Requires pypm, available at http://bitbucket.org/aalex/pyportmidi/wiki/Home])])

AX_PYTHON_MODULE([twisted], [Fatal])
AX_PYTHON_MODULE([gtk], [Fatal])
AX_PYTHON_MODULE([gst], [Fatal])
AX_PYTHON_MODULE([json], [])
AX_PYTHON_MODULE([simplejson], [])

AS_IF([test "x${HAVE_PYMOD_JSON}" = "xno"], 
    AS_IF([test "x${HAVE_PYMOD_SIMPLEJSON}" = "xno"],
        [AC_MSG_ERROR([Requires one of json or simplejson])]))

# check for programs
# Maybe $PATH does not contain /sbin
AC_CHECK_PROG(IFCONFIG, ifconfig, yes, no, "${PATH}:/sbin")
AS_IF([test "x${IFCONFIG}" = "xno"],
    [AC_MSG_ERROR([Requires ifconfig])])

AC_CHECK_PROG(XDPYINFO, xdpyinfo, yes, no)
AS_IF([test "x${XDPYINFO}" = "xno"],
    [AC_MSG_ERROR([Requires xdpyinfo])])

AC_CHECK_PROG(XVINFO, xvinfo, yes, no)
AS_IF([test "x${XVINFO}" = "xno"],
    [AC_MSG_ERROR([Requires xvinfo])])

AC_CHECK_PROG(JACKD, jackd, yes, no)
AS_IF([test "x${JACKD}" = "xno"],
    [AC_MSG_ERROR([Requires jackd])])

# Check for gst-plugins that we use
GST_CHECK=$srcdir/utils/gst_check.py
# PYTHON defined in ax_python_module
AS_IF([! $PYTHON $GST_CHECK], [AC_MSG_ERROR([Not all gstreamer plugins installed])])

if test "x${enable_debug}" = "xyes" ; then
    AC_DEFINE(CONFIG_DEBUG_LOCAL,[],[Debug define from configure])
    CFLAGS="-Wextra -pedantic -Wfatal-errors -Werror -Wall -Wno-long-long -Wno-variadic-macros -Wshadow"
    CXXFLAGS="-Weffc++ "
    CXXFLAGS+=$CFLAGS
fi

CXXFLAGS+=" -fPIC "

AM_CONDITIONAL([PROJ_DEBUG],test "x${enable_debug}" = "xyes")

# FIXME
SCENIC_LOCALE_DIR=locale
AC_SUBST(SCENIC_LOCALE_DIR)

AS_AC_EXPAND(LIBDIR, $libdir)
AC_MSG_NOTICE(Storing library files in $LIBDIR)

AS_AC_EXPAND(DATADIR, $datadir)
AC_MSG_NOTICE(Storing library files in $LIBDIR)

PIXMAPS_DIR=$DATADIR/scenic/pixmaps
AC_SUBST(PIXMAPS_DIR)

AS_AC_EXPAND(PKGLIBDIR, $pkglibdir)
AC_MSG_NOTICE(Storing package files in $PKGLIBDIR)

PYTHON_LIB_PATH=$LIBDIR/scenic/py
AC_SUBST(PYTHON_LIB_PATH)

AC_CONFIG_FILES([py/scripts/scenic], [chmod +x py/scripts/scenic])
AC_CONFIG_FILES([py/scripts/midistream], [chmod +x py/scripts/midistream])

AC_CONFIG_FILES([
Makefile
data/Makefile
data/glade/Makefile
data/pixmaps/Makefile
data/themes/Makefile
data/themes/Darklooks/Makefile
doc/Makefile
man/Makefile
utils/Makefile
utils/dc-ctl/Makefile
utils/jack-info/Makefile
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
locale/Makefile
locale/en_US/Makefile
locale/en_US/LC_MESSAGES/Makefile
locale/fr_CA/Makefile
locale/fr_CA/LC_MESSAGES/Makefile
py/scenic/test/Makefile
py/scripts/Makefile
src/include/Makefile
src/util/Makefile
src/gutil/Makefile
src/shared_video/Makefile
src/vumeter/Makefile
src/tcp/Makefile
src/gst/Makefile
src/firereset/Makefile
src/main/Makefile
src/main/test/Makefile
])

AC_OUTPUT
