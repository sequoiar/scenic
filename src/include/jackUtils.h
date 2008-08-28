// jackutils.h
// Copyright 2008 Koya Charles & Tristan Matthews
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

/** \file
 *      Utility functions for jack related issues.
 *
 *
 */

#ifndef _JACK_UTILS_H_
#define _JACK_UTILS_H_

#include <jack/jack.h>
#include <iostream>

static bool jack_is_running()
{
    jack_client_t *client;
    jack_status_t status;

    /* Open a client connection to the JACK server.  Starting a
     * new server only to see if it's running is pointless, so we
     * specify JackNoStartServer. */

    client = jack_client_open ("AudioJackSource", JackNoStartServer, &status);
    if (client == NULL) {
        if (status & JackServerFailed)
            std::cerr << "JACK server not running" << std::endl;
        else
            std::cerr << "jack_client_open() failed, status = 0x" << status <<
            std::endl;
        jack_client_close(client);
        return false;
    }
    jack_client_close(client);
    return true;
}


#endif //  _JACK_UTILS_H_
