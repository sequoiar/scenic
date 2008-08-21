#include "sipsession.h"

#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE( sip_export )
{
    class_<SIPSession>("SIPSession", init<int>())
    .def("connect", &SIPSession::connect)
    .def("addMedia", &SIPSession::addMedia)
    .def("disconnect", &SIPSession::disconnect)
    ;
}

