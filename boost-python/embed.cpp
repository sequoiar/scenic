#include <boost/python.hpp>
#include <stdio.h>
#include <iostream>
#include <string>

/*
 * Embedded Python example
 *  
 * Exporting C++ module to python requires 
 *
 * PyImport_AppendInittab()
 *
 * boost_python provides an init function 
 * called init{modulename}
 *
 * PyImport_AppendInittab((char*)"hello",&inithello);
 *
 * must come before PyInitialize()
 */

using namespace boost::python;

class hello
{
    std::string s;
public:
    const char * greet();
    void set_name(char const* n);


};

const char * hello::greet()
{
    static std::string ts;
    ts = "hello "+ s;
    
    return ts.c_str();

}

void hello::set_name(char const* n)
{
    s = n;
}



BOOST_PYTHON_MODULE(hello)
{
    class_<hello>("hello")
    .def("greet", &hello::greet)
    .def("set_name",&hello::set_name);
}


int main(int argc, char *argv[])
{
    object imp;
    
    // This line is needed for python to see the module hello
    PyImport_AppendInittab((char*)"hello",&inithello);
    Py_Initialize();

    object main_module = import("__main__");
    object main_namespace = main_module.attr("__dict__");

    try
    {
        if(main_module)
        {
            exec("import readline",main_namespace,main_namespace);
            exec("import hello; from hello import *",main_namespace,main_namespace);
            exec("from code import InteractiveConsole",main_namespace,main_namespace);		
            exec("i = InteractiveConsole(globals())",main_namespace,main_namespace);
            exec("i.interact()",main_namespace,main_namespace);
        }
        else
            printf("err\n");

    }
    catch(error_already_set const &)
    {
        PyErr_Print();

    }
}

// c++ -I /usr/include/python2.5 -I /usr/include/boost/python/ embed.cpp  /usr/lib/libboost_python-gcc42-1_34_1.a  /usr/lib/libpython2.5.so.1.0
