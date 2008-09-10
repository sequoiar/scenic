
// jackUtils.cpp
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

#include <jack/jack.h>
#include <jack/transport.h>
#include <iostream>
#include <cassert>
#include "jackUtils.h"
#include "logWriter.h"

bool Jack::is_running()
{
    jack_client_t *client;
    jack_status_t status;

    /* Open a client connection to the JACK server.  Starting a
     * new server only to see if it's running is pointless, so we
     * specify JackNoStartServer. */

    client = jack_client_open ("AudioJackSource", JackNoStartServer, &status);

    if (client == NULL) {
        if (status & JackServerFailed)
            LOG("JACK server not running", ERROR);
        else
            LOG("jack_client_open() failed, check status", ERROR); 

        jack_client_close(client);
        return false;
    }
    else
        return true;
#if 0

    jack_transport_state_t ts = jack_transport_query(client, NULL);
    jack_client_close(client);

    if (ts == JackTransportRolling)
        return true;
    else if (ts == JackTransportStopped)
        return false;
    else 
    {
        LOG("Unknown jack server state", ERROR);
        return false;
    }
#endif
}

