/* pyMsgThread.c
 * Copyright 2008 Koya Charles & Tristan Matthews 
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "pyMsgThread.h"

int PythonThread::main()
{
    LOG_INFO("Python Thread");
    for(;;)
    {
        MapMsg m = q_.timed_pop(10);
        if(m["command"].empty())
            continue;
        if(std::string(m["command"]) == "quit")
            break;

        PyGILState_STATE state = PyGILState_Ensure();
        LOG_INFO("makeDictionary");
        msgH_.cb(makeDict(m)); 
        PyGILState_Release(state);

    }
    return 0; 
}


