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
    .def("message", &SIPSession::send_instant_message)
    .def("set_media", &SIPSession::set_media, (arg("dir") = DEFAULT_PARAMETER))
    .def("media_to_string", &SIPSession::media_to_string)
    .def("state", &SIPSession::get_connection_state)
    .def("error_reason", &SIPSession::get_error_reason)
    .def("get_answer_mode", &SIPSession::get_answer_mode)
    .def("set_answer_mode", &SIPSession::set_answer_mode)
    .def("set_python_instance", &SIPSession::set_python_instance)
    ;
}

