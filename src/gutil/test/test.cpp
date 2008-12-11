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

#include "util.h"

#include "baseModule.h"
#include "gutil.h"

class DModule
    : public BaseModule
{
    public:
        DModule()
            : pass_(false){}
        bool run();
        void init();

    private:
        bool pass_;
};

void DModule::init()
{
//    args_.push_back(new BoolArg(&pass_, "pass", 'p', "bypass this module"));
}


bool DModule::run()
{
    if(!pass_) {
        LOG("x ", DEBUG);
    }
    return true;
}


int my_main(int argc, char** argv)
{
    DModule m;
    OptionArgs opts;
    m.init();
    if(!opts.parse(argc, argv))
        return 1;
    m.run();

    return 0;
}


int main (int argc, char** argv)
{
    return my_main(argc, argv);
}


