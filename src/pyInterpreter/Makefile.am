bin_PROGRAMS = pythonInterpreter
pythonInterpreter_SOURCES = pyInterpreter.cpp pyInterpreter_main.cpp
pythonInterpreter_LDADD = ../hello/libhello.a $(PYTHON_LIBS) $(GST_LIBS) $(GSTAUDIO_LIBS) 
pythonInterpreter_CPPFLAGS = -I../ $(PYTHON_CFLAGS) $(GST_CFLAGS) 

