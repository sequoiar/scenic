GST_LDFLAGS=-pthread -L/usr/local/lib -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -ldl -lgthread-2.0 -lrt -lxml2 -lglib-2.0  



all:
	c++ pyInterpreter_main.cpp pyInterpreter.cpp ../gst/mediaBase.o ../gst/videoSender.o ../gst/../hello/hello.o $(GST_LDFLAGS) -I /usr/include/python2.5 -I /usr/include/boost/python/ /usr/lib/libboost_python-gcc42-1_34_1.a  /usr/lib/libpython2.5.so.1.0 -o python_main

clean:
	rm embed

