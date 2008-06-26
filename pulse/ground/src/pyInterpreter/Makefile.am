bin_PROGRAMS = pythonInterpreter
pythonInterpreter_SOURCES = pyInterpreter.cpp pyInterpreter_main.cpp
pythonInterpreter_LDADD = ../thread/libBaseThread.a ../hello/libhello.a $(GLIB_LIBS) $(PYTHON_LIBS) $(GST_LIBS) $(GSTAUDIO_LIBS) 
pythonInterpreter_CPPFLAGS = -I../ $(PYTHON_CFLAGS) $(GST_CFLAGS) $(GLIB_CFLAGS)

