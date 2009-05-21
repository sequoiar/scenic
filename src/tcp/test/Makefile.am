include $(top_srcdir)/globals.mak

bin_PROGRAMS = parserTester

parserTester_SOURCES = parserTester.cpp 

#LDADD = $(LIBUTIL) $(src)/tcp/libtcp.a $(GLIB_LIBS) $(CPPTEST_LIBS) 
LDADD = $(CPPTEST_LIBS) $(GLIB_LIBS) $(src)/tcp/libtcp.a $(src)/gutil/libgutil.a $(LIBUTIL) -ldl
#parserTester_LDADD = $(LIBUTIL) $(src)/tcp/libtcp.a $(CPPTEST_LIBS) 


