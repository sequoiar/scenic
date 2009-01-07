#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(PROPULSE_ART, 0.1.2 )
AC_DEFINE(RELEASE_CANDIDATE,["-rc3"],[rc version num])
AC_CONFIG_HEADER([src/config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL

AM_INIT_AUTOMAKE()

#AM_CONFIG_HEADER(config.h)
# Check for C and C++ compilers
AC_PROG_CXX


AC_PROG_CC

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
PKG_PROG_PKG_CONFIG

AC_PROG_GCC_TRADITIONAL
AC_CHECK_FUNCS([inet_ntoa memset socket])

AC_MSG_CHECKING(whether to enable debugging)
AC_ARG_ENABLE([sip], [ **************** 
  --enable-sip build sip],,enable_sip=yes)
AC_ARG_ENABLE([debug], [
  --enable-debug  debug symbols, build test programs, stricter compile ],, 
        enable_debug=no)

AC_MSG_WARN([${ac_prefix}])
# Checks for library functions.

if test "x${enable_debug}" = "xyes" ; then
    AC_DEFINE(CONFIG_DEBUG_LOCAL,[],[Debug define from configure])
    CXXFLAGS="-g -O0 -Wextra -Weffc++ -pedantic -Wfatal-errors -Werror -Wno-variadic-macros -Wno-long-long -Wall -Wshadow"

Trying to set CPPUNIT_LIBS using pkg-config
PKG_CHECK_MODULES(CPPUNIT, cppunit, have_cppunit=true, have_cppunit=false)
if test "x${have_cppunit}" = "xfalse" ; then
    AC_MSG_WARN([libcppunit not found.-- ])
fi

fi

CXXFLAGS+=" -fPIC "

#Trying to set GLIB using pkg-config
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

#Trying to set JACK using pkg-config
PKG_CHECK_MODULES(JACK, jack >= 0.99.10, have_jack=true, have_jack=false)
if test "x${have_jack}" = "xfalse" ; then
    AC_MSG_ERROR([missing pkg -  libjack0 and/or libjack-dev])
fi

#Trying to set LIBAVC1394 using pkg-config
PKG_CHECK_MODULES(LIBAVC1394, [libavc1394], have_libavc1394=true, have_libavc1394=false)
if test "x${have_libavc1394}" = "xfalse" ; then
	AC_MSG_WARN([missing pkg - libavc1394-dev ])
fi

#Trying to set LIBAVC1394 using pkg-config
PKG_CHECK_MODULES(LIBRAW1394, [libraw1394], have_libraw1394=true, have_libraw1394=false)
if test "x${have_libraw1394}" = "xfalse" ; then
	AC_MSG_WARN([missing pkg - libraw1394-dev ])
fi

AC_SUBST(ALL_GST_LIBS, "$GST_LIBS $GSTAUDIO_LIBS")
#AC_SUBST(ALL_GST_LIBS)

if test "x${enable_sip}" = "xyes" ; then

#Trying to set PJSIP using pkg-config
PKG_CHECK_MODULES(UUID, uuid, have_uuid=true, have_uuid=false)
if test "x${have_libuuid}" = "xfalse" ; then
    AC_MSG_WARN([UUID not found. pkg libuuid1])
fi

#Trying to set PJSIP using pkg-config
PKG_CHECK_MODULES(PJSIP, libpj, have_libpj=true, have_libpj=false)
if test "x${have_libpj}" = "xfalse" ; then
    AC_MSG_WARN([PJSIP not found. http://www.pjsip.org/download.htm])
fi
fi

#Trying to set LIBLO using pkg-config
#PKG_CHECK_MODULES(LIBLO, liblo >= 0.25, have_liblo=true, have_liblo=false)
#if test "x${have_liblo}" = "xfalse" ; then
#    AC_MSG_WARN([LIBLO missing. pkg - liblo0,liblo0-dev ])
#fi
#AM_CONDITIONAL([OSC],test "x${have_liblo}" == "xyes" )

#PYTHON_CFLAGS="-I/usr/include/boost/python "
#PYTHON_CFLAGS+=`python-config --includes` 
#PYTHON_LIBS="/usr/lib/libboost_python.so "
#PYTHON_LIBS+=`python-config --ldflags` 

#AC_SUBST(PYTHON_CFLAGS)
#AC_SUBST(PYTHON_LIBS)

AX_PYTHON
AM_PATH_PYTHON(2.4, have_python=true,have_python=false)
AX_BOOST_PYTHON
AM_CONDITIONAL([BOOST_PYTHON], test "$ac_cv_boost_python" = "yes")

AX_CHECK_GL
AX_CHECK_GLU

if test "x${no_gl}" = "xyes" ; then
    AC_MSG_WARN([Missing OpenGL try some of mesa-common-dev libgl1-mesa-swx11-dev, xorg-driver-fglrx-dev nvidia-glx-new-dev.])
fi

AX_CHECK_GLU
if test "x${no_glu}" = "xyes" ; then
    AC_MSG_WARN([Missing GLU install libglu1-mesa-dev])
else
    AC_DEFINE(CONFIG_GL,[],[GL define from configure])
fi

#
#
#
#
AM_CONDITIONAL([SIP],test "x${enable_sip}" = "xyes" -a "x{have_libpj}" = "xtrue" )
#AM_CONDITIONAL([IEEE1394],test "x{have_libavc1394}" = "xtrue" -a "x{have_libraw1394}" = "xtrue" )
AM_CONDITIONAL([PROJ_DEBUG],test "x${enable_debug}" = "xyes" )

if test "x${have_libavc1394}" = "xfalse" -o "x${have_libavc1394}" = "xfalse" ; then
AC_MSG_NOTICE([undef CONFIG_IEEE1394])
else
AC_DEFINE(CONFIG_IEEE1394,[],[IEEE1934 define from configure])
AC_MSG_NOTICE([define CONFIG_IEEE1394])
fi
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/util/Makefile
                 src/gutil/Makefile
                 src/tcp/Makefile
                 src/gst/Makefile
                 src/gst/engine/Makefile
                 src/gst/control/Makefile
                 src/main/Makefile
                 src/python/Makefile
                 src/sip/Makefile
                 ])

#if test  "x${enable_debug}" = "xyes"  -a  "x${have_cpptest}" = "xtrue"  ; then

#                 src/util/test/Makefile
#                 src/gutil/test/Makefile
#                 src/tcp/test/Makefile
#                 src/sip/test/Makefile

AC_CONFIG_FILES([
                 src/gst/engine/test/Makefile
                 src/gst/engine/test.old/Makefile
                 ])


#fi
AC_OUTPUT
echo

if test  "$ac_cv_boost_python" != "yes"   ; then
AC_MSG_NOTICE([Not building python directories -- python or boost-python not found ])
fi

#if test  "x${enable_debug}" = "xyes" -a "x${have_cpptest}" = "xfalse"  ; then
#AC_MSG_NOTICE([Not building test directories -- no cpptest found])
#fi

if test  "x${enable_sip}" = "xyes" -a "x${have_libpj}" = "xfalse"  ; then
AC_MSG_NOTICE([Not building sip directory -- libpj missing])
fi

if test "x${have_libavc1394}" = "xfalse" -o "x${have_libavc1394}" = "xfalse" ; then
AC_MSG_NOTICE([Not building 1394 support])
fi

if test "x${no_glu}" = "xyes" ; then
AC_MSG_NOTICE([Not building OpenGL support])
fi

