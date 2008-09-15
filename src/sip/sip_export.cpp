#include "sipsession.h"

#include <boost/python.hpp>

using namespace boost::python;

/*
 * C++ wrapper for python
 * These methods are callable from python
 */
BOOST_PYTHON_MODULE( libsip_export )
{
    class_<SIPSession>("SIPSession", init<int>())
    .def(init<>())
    .def("connect", &SIPSession::connect, (arg("r_uri") = DEFAULT_PARAMETER))
    .def("accept", &SIPSession::accept)
    .def("refuse", &SIPSession::refuse)
    .def("disconnect", &SIPSession::disconnect)
    .def("reinvite", &SIPSession::reinvite)
    .def("shutdown", &SIPSession::shutdown)
    .def("init", &SIPSession::init)
    .def("message", &SIPSession::sendInstantMessage)
    .def("setMedia", &SIPSession::setMedia, (arg("dir") = DEFAULT_PARAMETER))
    .def("mediaToString", &SIPSession::mediaToString)
    .def("state", &SIPSession::getConnectionState)
    .def("errorReason", &SIPSession::getErrorReason)
    .def("getAnswerMode", &SIPSession::getAnswerMode)
    .def("setAnswerMode", &SIPSession::setAnswerMode)
    .def("getFinalCodec", &SIPSession::getFinalCodec)
    ;
}

