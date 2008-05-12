#include <boost/python.hpp>


#include "../hello/hello.h"

#define MODULE_STR  "hello"
#define MODULE_INIT      inithello

BOOST_PYTHON_MODULE(hello)
{
    class_<hello>("hello")
    .def("greet", &hello::greet)
    .def("set_name",&hello::set_name);
}


