#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(PROPULSE_ART, 0.01 )
AC_CONFIG_SRCDIR([src/gst/mediaBase.h])
AC_CONFIG_HEADER([src/config.h])
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


# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_CHECK_FUNCS([inet_ntoa memset socket])

#Trying to set CPPTEST_LIBS using pkg-config
PKG_CHECK_MODULES(CPPTEST, libcpptest, have_cpptest=true, have_cpptest=false)
if test "x${have_cpptest}" = "xfalse" ; then
    AC_MSG_ERROR([libcpptest not found.-- ubuntu: sudo apt-get install libcpptest-dev])
fi


#Trying to set CPPTEST_LIBS using pkg-config
PKG_CHECK_MODULES(GLIB, [glib-2.0 gthread-2.0], have_glib=true, have_glib=false)
if test "x${have_glib}" = "xfalse" ; then
    AC_MSG_ERROR([glib not found.])
fi

#Trying to set GST_CFLAGS GST_LIBS using pkg-config
PKG_CHECK_MODULES(GST, gstreamer-0.10, have_gstreamer=true, have_gstreamer=false)
if test "x${have_gstreamer}" = "xfalse" ; then
    AC_MSG_ERROR([GStreamer not found.])
fi


#Trying to set GSTAUDIO using pkg-config
PKG_CHECK_MODULES(GSTAUDIO, gstreamer-audio-0.10, have_gstreamer_audio=true, have_gstreamer_audio=false)
if test "x${have_gstreamer_audio}" = "xfalse" ; then
    AC_MSG_ERROR([missing pkg -  libgstreamer-plugins-base0.10-dev  ])
fi

#Trying to set GSTCONTROLLER using pkg-config
PKG_CHECK_MODULES(GSTCONTROLLER, gstreamer-controller-0.10, have_gstreamer_controller=true, have_gstreamer_controller=false)
if test "x${have_gstreamer_controller}" = "xfalse" ; then
    AC_MSG_ERROR([missing pkg -  libgstreamer-plugins-base0.10-dev  ])
fi

#Trying to set PJSIP using pkg-config
PKG_CHECK_MODULES(UUID, uuid, have_uuid=true, have_uuid=false)
if test "x${have_libuuid}" = "xfalse" ; then
    AC_MSG_ERROR([UUID not found. pkg libuuid1])
fi

#Trying to set PJSIP using pkg-config
PKG_CHECK_MODULES(PJSIP, libpj, have_libpj=true, have_libpj=false)
if test "x${have_libpj}" = "xfalse" ; then
    AC_MSG_ERROR([PJSIP not found. http://www.pjsip.org/download.htm])
fi

#Trying to set LIBLO using pkg-config
PKG_CHECK_MODULES(LIBLO, liblo, have_liblo=true, have_liblo=false)
if test "x${have_liblo}" = "xfalse" ; then
    AC_MSG_ERROR([LIBLO missing. pkg - liblo0,liblo0-dev ])
fi

#PYTHON_CFLAGS="-I/usr/include/boost/python "
#PYTHON_CFLAGS+=`python-config --includes` 
#PYTHON_LIBS="/usr/lib/libboost_python.so "
#PYTHON_LIBS+=`python-config --ldflags` 

#AC_SUBST(PYTHON_CFLAGS)
#AC_SUBST(PYTHON_LIBS)

AX_PYTHON
AX_BOOST_PYTHON



AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/gst/Makefile
                 src/gst/test/Makefile
                 src/sip/Makefile
                 src/sip/test/Makefile
                 src/gutil/Makefile
                 src/gutil/test/Makefile
                 src/osc/Makefile
                 src/osc/test/Makefile
                 src/main/Makefile
                 src/prototypes/Makefile
                 src/prototypes/hello/Makefile
                 src/prototypes/hello/test/Makefile
                 src/prototypes/python/Makefile
                 ])
AC_OUTPUT

