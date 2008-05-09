AC_PREREQ(2.59)
AC_INIT

AM_INIT_AUTOMAKE(camera, 0.1.0)

AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_AS

CFLAGS="$CFLAGS -Wall -Wmissing-prototypes -Wmissing-declarations"

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h locale.h memory.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_STDBOOL
AC_TYPE_UID_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_SETVBUF_REVERSED
AC_CHECK_FUNCS([atexit memset setlocale])

dnl Check for gstreamer
GST_REQUIRED=0.10.0
GST_MAJORMINOR=0.10

PKG_CHECK_MODULES(GST, \
  gstreamer-$GST_MAJORMINOR >= $GST_REQUIRED
  gstreamer-plugins-base-$GST_MAJORMINOR >= $GST_REQUIRED,
  HAVE_GST=yes,HAVE_GST=no)

dnl Give error and exit if we don't have gstreamer
if test "x$HAVE_GST" = "xno"; then
  AC_MSG_ERROR(you need gstreamer development packages installed !)
else
  GST_LIBS="$GST_LIBS -lgstinterfaces-$GST_MAJORMINOR"
fi

dnl make GST_CFLAGS and GST_LIBS available
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_LIBS)

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.2.0 hildon-lgpl >= 0.8.15)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(HILDON, hildon-libs >= 0.8.4 hildon-fm libosso >= 0.8.1 libossohelp >= 1.0.22-4 )
AC_SUBST(HILDON_CFLAGS)
AC_SUBST(HILDON_LIBS)

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.2)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(GNOMEVFS, gnome-vfs-2.0 >= 2.8.0)
AC_SUBST(GNOMEVFS_CFLAGS)
AC_SUBST(GNOMEVFS_LIBS)

AC_CONFIG_FILES([
Makefile
src/Makefile
data/Makefile
])
AC_OUTPUT
