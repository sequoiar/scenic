#include <glib.h>
#include <cstring>
#include <cstdlib>
#include "Python.h"

#include "hello.h"

const char *Hello::greet()
{
    static std::string ts;

    ts = "hello " + s;

    return ts.c_str();
}

int i = 0;
gpointer gMain(gpointer )
{
    while(1)
    {
        usleep(1000000);
        {
            PyGILState_STATE state = PyGILState_Ensure();
            char t[100];
            sprintf(t,"print %d",i);
            PyRun_SimpleString(t);

            PyGILState_Release(state);


        }
    }

}


void Hello::set_name(char const *n)
{
    s = n;
    g_thread_init(0);

    PyEval_InitThreads();
    i = atoi(n);
    g_thread_create(gMain,0,0,0);
}


