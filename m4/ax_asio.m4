# ===========================================================================
#             http://autoconf-archive.cryp.to/ax_boost_asio.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_ASIO
#
# DESCRIPTION
#
#   Test for Asio library from the Boost C++ libraries. The macro requires a
#   preceding call to AX_BOOST_BASE. Further documentation is available at
#   <http://randspringer.de/boost/index.html>.
#
#   This macro calls:
#
#     AC_SUBST(BOOST_ASIO_LIB)
#
#   And sets:
#
#     HAVE_BOOST_ASIO
#
# LAST MODIFICATION
#
#   2008-04-12
#
# COPYLEFT
#
#   Copyright (c) 2008 Thomas Porschberg <thomas@randspringer.de>
#   Copyright (c) 2008 Pete Greenwell <pete@mu.org>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

AC_DEFUN([AX_ASIO],
[
        AC_REQUIRE([AC_PROG_CC])
		CPPFLAGS_SAVED="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
		export CPPFLAGS

		LDFLAGS_SAVED="$LDFLAGS"
		LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
		export LDFLAGS

        AC_CACHE_CHECK(whether the ASIO library is available,
					   ax_cv_asio,
        [AC_LANG_PUSH([C++])
		 AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[ @%:@include <asio.hpp>
											]],
                                  [[

                                    asio::io_service io;
                                    asio::error_code timer_result;
                                    asio::deadline_timer t(io);
                                    t.cancel();
                                    io.run_one();
									return 0;
                                   ]]),
                             ax_cv_asio=yes, ax_cv_asio=no)
         AC_LANG_POP([C++])
		])
		if test "x$ax_cv_asio" = "xyes"; then
			AC_DEFINE(HAVE_ASIO,,[define if the ASIO library is available])
                fi

		CPPFLAGS="$CPPFLAGS_SAVED"
    	LDFLAGS="$LDFLAGS_SAVED"
	])


