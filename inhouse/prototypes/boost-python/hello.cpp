#include "hello.h"
#include <boost/python.hpp>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;
using namespace boost::python;


const char * hello::greet()
{
    static std::string ts;
    ts = "hello " + s;
    
    return ts.c_str();
}

void hello::set_name(char const* n)
{
    s = n;
}

void hello::printTest1()
{
    //Py_Initialize();
    try
    {
        std::ostringstream os;
    
        os << "import pycoreTest as pycore\n" ;
        os << "msg = 'salut'\n"; 
        os << "pycore.pyPrint(msg)\n";

        PyRun_SimpleString(os.str().c_str());
    }
    
    catch(error_already_set const &)
    {
        PyErr_Print();
    }
    
    //Py_Finalize();

}

void hello::printTest2()
{
    //Py_Initialize();
    try
    {
        std::ostringstream os;
    
        os << "import pycoreTest as pycore\n" ;
        os << "msg = 'resalut'\n"; 
        os << "pycore.pyPrint(msg)\n";

        PyRun_SimpleString(os.str().c_str());
    }
    
    catch(error_already_set const &)
    {
        PyErr_Print();
    }
    
    //Py_Finalize();

}
