bin_PROGRAMS = pythonInterpreter
pythonInterpreter_SOURCES = pyInterpreter.cpp pyInterpreter_main.cpp
pythonInterpreter_LDADD = ../osc/libOsc.a ../hello/libhello.a $(LIBLO_LIBS) $(GLIB_LIBS) -l$(BOOST_PYTHON_LIB) -l$(PYTHON_LIB)  $(GST_LIBS) $(GSTAUDIO_LIBS) 
pythonInterpreter_CPPFLAGS = -I../ -I$(PYTHON_INCLUDE_DIR) $(GST_CFLAGS) $(GLIB_CFLAGS)

