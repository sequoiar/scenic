#include "osc.h"
#include <iostream>

#include <list>
#include <vector>

int main(int argc,char** argv)
{
   OscThread t;
 
   QueuePairOfOscMessage q = t.getQueue("");
   QueuePairOfOscMessage q2 = t.getQueue("/allo");

    std::cout << q.first << "QQ" << q.second << std::endl;
    std::cout << q2.first << "2QQ2" << q2.second << std::endl;


   t.run();

   while(1)
   {
        OscMessage m = q.copy_timed_pop(1000);
        OscMessage m2 = q2.copy_timed_pop(1); 
        if(!m.path.empty())                  
		{
            std::cout <<"main"<< m.path << m.args[0].s << m.args[1].i << std::endl; 
            if(m.path == "/echo")
                q.push(m);
		}

        if(!m2.path.empty())
            std::cout <<"allo queue !"<< m.path << m.args[0].s << m.args[1].i << std::endl; 
    }

}

