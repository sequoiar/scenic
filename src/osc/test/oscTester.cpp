#include "osc.h"
#include <iostream>

#include <list>
#include <vector>

int main(int argc,char** argv)
{
   OscThread t;
 
   QueuePairOfOscMessage q = t.getInvertQueue();

   t.run();

   while(1)
   {
        OscMessage m = q.copy_timed_pop(1000);
 
        if(!m.path.empty())                  
		{
            std::cout <<"main"<< m.path << m.args[0].s << m.args[1].i << std::endl; 
		}

    }

}

