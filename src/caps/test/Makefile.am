TESTS = caps_test
check_PROGRAMS = caps_test

caps_test_SOURCES = caps_test.cpp $(top_builddir)/src/caps/caps_server.h $(top_builddir)/src/caps/caps_server.cpp \
                    $(top_builddir)/src/caps/caps_client.h $(top_builddir)/src/caps/caps_client.cpp
caps_test_LDADD = $(top_builddir)/src/caps/libcaps.la $(top_builddir)/src/util/libutil.la $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)
INCLUDES = -I$(top_srcdir)/src/include
