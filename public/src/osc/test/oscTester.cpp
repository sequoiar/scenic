#include "osc.h"
#include <iostream>

#include <list>
#include <vector>

int main(int argc, char **argv)
{
    OscThread t;

    QueuePairOfOscMessage q = t.getQueue();

    t.run();

    while (1)
    {
        OscMessage m = q.timed_pop(1);

        if (!m.path.empty())
            std::cout << m.path << m.args[0].s << m.args[1].i << std::endl;
        if (!m.path.compare("/echo"))
            q.push(m);
    }
}

