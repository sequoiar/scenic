#include "osc.h"
#include <iostream>
#include <string.h>

#include <list>
#include <vector>

int main(int, char **)
{
    OscThread t;

    OscQueue &q = t.getQueue();

    t.run();

    while (1)
    {
        OscMessage m = q.timed_pop(1);

        if (m.pathIsSet())
            m.print();
        if (!m.path().compare(0, 5, "/echo"))
            q.push(m);
    }
}


