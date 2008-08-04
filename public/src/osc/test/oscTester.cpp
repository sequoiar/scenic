#include "osc.h"
#include <iostream>
#include <string.h>

#include <list>
#include <vector>

int main(int argc, char **argv)
{
    OscThread t;

    OscQueue &q = t.getQueue();

    t.run();

    while (1)
    {
        OscMessage m = q.timed_pop(1);

        if (m.pathIsSet())
            m.print();
        //std::cout << m.path() << m.args[0].s << m.args[1].i << std::endl;
        if (!strncmp(m.path(), "/echo", strlen("/echo")))
            q.push(m);
    }
}


