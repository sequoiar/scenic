/* GTHREAD-QUEUE-PAIR - Library of Thread Queue Routines for GLIB
 * Copyright (C) 2008	Koya Charles, Tristan Matthews
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
#include <boost/signal.hpp>
#include "gThreadQueue.h"

struct foo
{
    foo(int i) : x(i)
    {
    }
    void s(int y)
    {
        x = y;
    }
    int x;
    void operator() ()
    {
        std::cout << (long) g_thread_self() << "---op()" << x << std::endl;
    }
};

typedef boost::signal < void () > Sig;

void *thread_main(void *v)
{
    QueuePair & queue = *(static_cast < QueuePair * >(v));

    foo f(18);
    int count = 0;
    while (1)
    {
        Sig & signal = *queue_pair_pop < Sig * >(queue);

        signal();
        queue_pair_push(queue, &f);
        if (count++ == 1000) {
            static foo f(0);
            queue_pair_push(queue, &f);
            return 0;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    GError *err = 0;
    Sig sig;
    foo f(10);
    sig.connect(f);

    g_thread_init(NULL);

    QueuePair queue(g_async_queue_new(), g_async_queue_new());
    InvertQueuePair tq(&queue);
    GThread *th = thread_create_queue_pair(thread_main, &tq, &err);

    while (err == NULL)
    {
        queue_pair_push(queue, &sig);
        sig();
        if (foo * f = queue_pair_timed_pop < foo * >(queue, 10)) {
            if (f->x == 0)
                break;
            std::cout << ":LKJ" << f->x;
        }

        f.x++;
        std::cout << "sent it";
    }
    g_async_queue_unref(queue.first);
    g_async_queue_unref(queue.second);

    g_thread_join(th);
    return 0;
}
