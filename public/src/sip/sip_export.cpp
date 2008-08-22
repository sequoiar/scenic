#include "sipsession.h"

#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE( libsip_export )
{
    class_<SIPSession>("SIPSession", init<int>())
    .def("connect", &SIPSession::connect)
    .def("disconnect", &SIPSession::disconnect)
    .def("reinvite", &SIPSession::reinvite)
    .def("shutdown", &SIPSession::shutdown)
    .def("sendInstantMessage", &SIPSession::sendInstantMessage)
    .def("addMedia", &SIPSession::addMedia)
    .def("mediaToString", &SIPSession::mediaToString)
    ;
}

