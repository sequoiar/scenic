#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(MILHOUSE, 0.2)
AC_DEFINE(RELEASE_CANDIDATE,["-c1"],[rc version num])
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
#AC_ARG_ENABLE([sip], [ **************** 
#  --enable-sip build sip],,enable_sip=yes)
AC_ARG_ENABLE([debug], [
  --enable-debug  debug symbols, build test programs, stricter compile ],, 
        enable_debug=yes)

AC_ARG_ENABLE([millisecwait], [ --enable-millisecwait=(millisecs) thread sleep time],,enable_millisecwait=10)
AC_DEFINE_UNQUOTED(MILLISEC_WAIT,[${enable_millisecwait}], wait time)

AC_ARG_ENABLE([svn-revision], [ --enable-svn-revision ],,enable_svn_revision=no)
AM_CONDITIONAL([SVN],test "x${enable_svn_revision}" = "xyes")
AC_ARG_ENABLE([logs], [ --enable-logs enable logs in msg queues],,enable_logs=yes)
if test "x${enable_logs}" = "xyes" ; then
    AC_DEFINE(MAPLOGS,[],[send logs to listeners])
fi

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
PKG_CHECK_MODULES(LIBDC1394, [libdc1394-2], have_libdc1394-2=true, have_libdc1394-2=false)
if test "x${have_libdc1394-2}" = "xfalse" ; then
	AC_MSG_ERROR([missing pkg - libdc1394-2])
fi

AC_SUBST(ALL_GST_LIBS, "$GST_LIBS $GSTAUDIO_LIBS $GSTAPP_LIBS")

AX_PYTHON

# BOOST m4 tests
AX_BOOST_BASE([1.35])
AX_BOOST_THREAD
AX_BOOST_SYSTEM
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_ASIO
AX_BOOST_DATE_TIME
AX_BOOST_UNIT_TEST_FRAMEWORK
if test "x$with_boost_python" != "x" ; then 
    AM_PATH_PYTHON(2.4, have_python=true,have_python=false)
    if test "x$with_boost_python" = "xyes" ; then
        with_boost_python=no
    fi
    LDFLAGS_SAVE=$LDFLAGS
    LDFLAGS=-l$PYTHON_LIB $LDFLAGS
    AX_BOOST_PYTHON
    if test "x$BOOST_PYTHON_LIB" = "x" ; then
    	if test "x$with_boost_python" = "xno" ; then
           with_boost_python=mt
	else
	   with_boost_python=$with_boost_python-mt
	fi
	AX_BOOST_PYTHON
    fi
    LDFLAGS=$LDFLAGS_SAVE
fi



AM_CONDITIONAL([BOOST_PYTHON], test "$ac_cv_boost_python" = "yes")

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
AC_CONFIG_FILES([Makefile
		 doc/Makefile
		 doc/notes/Makefile
		 doc/test/Makefile
		 doc/network/Makefile
		 doc/figures/Makefile
		 doc/figures/test/Makefile
		 doc/sequences/Makefile
                 src/Makefile
                 py/Makefile
                 py/docs/Makefile
                 py/maugis/Makefile
                 py/prism/Makefile
                 py/miville/Makefile
                 py/miville/connectors/Makefile
                 py/miville/connectors/basic/Makefile
                 py/miville/connectors/sip/Makefile
                 py/miville/devices/Makefile
                 py/miville/protocols/Makefile
                 py/miville/streams/Makefile
                 py/miville/ui/Makefile
                 py/miville/utils/Makefile
                 src/include/Makefile
                 src/util/Makefile
                 src/gutil/Makefile
                 src/boost/Makefile
                 src/tcp/Makefile
                 src/gst/Makefile
                 src/gst/engine/Makefile
                 src/gst/control/Makefile
                 src/devices/Makefile
                 src/main/Makefile
                 src/python/Makefile
                 ])

AC_CONFIG_FILES([
                 src/util/test/Makefile
                 src/tcp/test/Makefile
                 src/gst/engine/test/Makefile
                 ])


AC_OUTPUT
echo

if test  "$ac_cv_boost_python" == "yes"   ; then
AC_MSG_NOTICE([Building python module])
fi

if test "x${have_libavc1394}" = "xfalse" -o "x${have_libavc1394}" = "xfalse" ; then
AC_MSG_NOTICE([Not building 1394 support])
fi

if test "x${no_glu}" = "xyes" ; then
AC_MSG_NOTICE([Not building OpenGL support])
fi

