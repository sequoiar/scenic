/* GTHREAD-QUEUE-PAIR - Library of Thread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <glib.h>
#include <iostream>
#include "baseThread.h"
#include "message.h"

typedef QueuePair_ < Message > QueuePair;

class Thread:public BaseThread < Message >
{
    int main();
};

int Thread::main()
{
    Message r(message::ok);
    int count = 0;
    while (1)
    {
        Message f = queue.copy_timed_pop(1);
        std::cout << message::str[f.type];
        queue.push(r);
        if (count++ == 100000)
        {
            Message f(message::quit);
            queue.push(f);
            break;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    Message f(message::start);
    Thread t;

    QueuePair queue = t.getQueue("");
    QueuePair queue2 = t.getQueue("a");
    if (!t.run())
        return -1;

    while (1)
    {
        queue.push(f);
        std::cout << "sent it" << std::endl;
        Message f = queue.copy_timed_pop(10);
        Message f2 = queue2.copy_timed_pop(10);

        std::cout << message::str[f.type];
        if (f.type == message::quit)
        {
            break;
        }

    }

    std::cout << "Done!" << std::endl;

    return 0;
}
