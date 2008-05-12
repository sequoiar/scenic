#include <boost/python.hpp>
#include <stdio.h>


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



#include "python_module_inc.cpp"



int main(int argc, char *argv[])
{
    object imp;
    
    // This line is needed for python to see the module hello
    PyImport_AppendInittab((char*)MODULE_STR,&MODULE_INIT);
    Py_Initialize();

    object main_module = import("__main__");
    object main_namespace = main_module.attr("__dict__");

    try
    {
        if(main_module)
        {
            exec("import readline",main_namespace,main_namespace);
            exec("import " MODULE_STR "; from " MODULE_STR " import *",main_namespace,main_namespace);
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
