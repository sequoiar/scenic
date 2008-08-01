src = $(top_srcdir)/src
INCLUDES = -I$(src)/include -I$(src) -I..
AM_CPPFLAGS = $(GLIB_CFLAGS) -O0 -g -Werror -Wall -Weffc++ -Wfatal-errors
#-Weffc++
