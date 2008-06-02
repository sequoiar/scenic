#include <boost/python.hpp>


#include "../hello/hello.h"

using namespace boost::python;


BOOST_PYTHON_MODULE(hello)
{
    class_<hello>("hello")
    .def("greet", &hello::greet)
    .def("set_name",&hello::set_name);
}

void BOOST_PY_IMPORT()
{
    PyImport_AppendInittab((char*)"hello",&inithello);
}

void PYTHON_EXEC_IMPORT(object mm, object mn)
{
    if(mm)
    {
        exec("import hello; from hello import *",mm,mn);
    }
}

