/* main.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful, * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include "milhouse.h"
#include "util.h"

int main(int argc, char **argv)
{
    int ret = 0;

    try
    {
        signal_handlers::setHandlers();
        Milhouse milhouse;
        ret = milhouse.run(argc, argv);
    }
    catch (const Except &e)  // these are our exceptions
    {
        if (std::string(e.what()).find("INTERRUPTED") not_eq std::string::npos)
        {
            std::cout << "Interrupted" << std::endl;
            ret = 0;
        }
        else
        {
            std::cerr << "exitting with error: " << e.what() << std::endl;
            std::cerr << "Please file a bug at " << PACKAGE_BUGREPORT << std::endl;
            ret = 1;
        }
    }
    catch (const std::exception &e)  // these are other exceptions (not one of our exception classes)
    {
        std::cerr << "exitting with error: " << e.what() << std::endl;
        std::cerr << "Please file a bug at " << PACKAGE_BUGREPORT << " if needed" << std::endl;
        ret = 1;
    }
    return ret;
}
