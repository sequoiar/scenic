all:
	c++ -I /usr/include/python2.5 -I /usr/include/boost/python/ -c embed.cpp hello.cpp 
	c++ -shared -o embed.so embed.o hello.o  /usr/lib/libboost_python-gcc42-1_34_1.a  /usr/lib/libpython2.5.so.1.0

clean:
	rm hello.o embed.o embed.so
