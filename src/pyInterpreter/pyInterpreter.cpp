
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

#include "pyInterpreter.h"


#include "../python_import.cpp"

static object main_module;
static object main_namespace;


void pyInterpreter::interact()
{
    exec("i.interact()",main_namespace,main_namespace);
}

std::string pyInterpreter::run_str(std::string s)
{
    std::string t_str = "i.push('";
    t_str += s;
    t_str += "')";
    object ret = eval(t_str.c_str(),main_namespace,main_namespace);
//BROKEN
    extract<char const*> get_str(ret);
    if(get_str.check())
        return std::string(get_str());
    else
    return std::string("");
}

std::string pyInterpreter::run_input()
{
try
{
    exec("i.push(i.raw_input('" PROMPT "'))",main_namespace,main_namespace);
}
catch(boost::python::error_already_set)
{    
    return std::string("EOF");
}
    return std::string("");
}


int pyInterpreter::init(int argc, char *argv[])
{
    object imp;
    
    // This line is needed for python 
    BOOST_PY_IMPORT();

    Py_Initialize();

    main_module = import("__main__");
    main_namespace = main_module.attr("__dict__");

    try
    {
        if(main_module)
        {
            exec("import readline",main_namespace,main_namespace);
            exec("from code import InteractiveConsole",main_namespace,main_namespace);	
            PYTHON_EXEC_IMPORT(main_namespace,main_namespace);
            exec("i = InteractiveConsole(globals())",main_namespace,main_namespace);
        }
        else
            PyErr_Print();

    }
    catch(error_already_set const &)
    {
        PyErr_Print();

    }

    return 0;
}

// c++ -I /usr/include/python2.5 -I /usr/include/boost/python/ embed.cpp  /usr/lib/libboost_python-gcc42-1_34_1.a  /usr/lib/libpython2.5.so.1.0
