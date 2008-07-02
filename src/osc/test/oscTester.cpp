#include "osc.h"
#include <iostream>

#include <list>
#include <vector>

int main(int argc, char **argv)
{
    OscThread t;

    QueuePairOfOscMessage q = t.getQueue("/allo");
    QueuePairOfOscMessage q3 = t.getQueue("/echo");

    t.run();

    while (1)
    {
        OscMessage m3 = q3.copy_timed_pop(20);
        OscMessage m = q.copy_timed_pop(1);

        if (!m.path.empty())
            std::cout << m.path << m.args[0].s << m.args[1].i << std::endl;
        if (!m3.path.empty())
            q3.push(m3);
    }

}
