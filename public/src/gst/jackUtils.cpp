
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
#include "jackUtils.h"
#include "logWriter.h"
#include <iomanip>

bool Jack::is_running()
{
    jack_client_t *client;
    jack_status_t status;

    /* Open a client connection to the JACK server.  Starting a
     * new server only to see if it's running is pointless, so we
     * specify JackNoStartServer. */

    client = jack_client_open ("AudioJackSource", JackNoStartServer, &status);

    if (client == NULL && (status & JackServerFailed))
        THROW_ERROR("JACK server not running");
    if (client == NULL) 
    {
        switch (status)
        {
            case JackFailure:   
                THROW_ERROR("Overall operation failed.");
                break;
           case JackInvalidOption:   
                THROW_ERROR("The operation contained an invalid or unsupported option.");
                break;
           case JackNameNotUnique:  
                THROW_ERROR("The desired client name was not unique. With the JackUseExactName option "
                            "this situation is fatal. Otherwise, the name was modified by appending a "
                            "dash and a two-digit number in the range -01 to -99. The jack_get_client_name()" 
                            "function will return the exact string that was used. If the specified client_name "
                            "plus these extra characters would be too long, the open fails instead.");
                break;
           case JackServerStarted:
                THROW_ERROR("The JACK server was started as a result of this operation. Otherwise, it was "
                        "running already. In either case the caller is now connected to jackd, so there is no race condition."
                        " When the server shuts down, the client will find out.");
                break;
           case JackServerFailed:   
                THROW_ERROR("Unable to connect to the JACK server.");
                break;
           case JackServerError:     
                THROW_ERROR("Communication error with the JACK server.");
                break;
            case JackNoSuchClient:    
                THROW_ERROR("Requested client does not exist."); 
                break;
            case JackLoadFailure:     
                THROW_ERROR("Unable to load internal client");
                break;
            case JackInitFailure:     
                THROW_ERROR("Unable to initialize client");
                break;
            case JackShmFailure:  
                THROW_ERROR("Unable to access shared memory");
                break;
            case JackVersionError:   
                THROW_ERROR("Client's protocol version does not match");
                break;
            default:
                THROW_ERROR("Overall operation mysteriously failed.");
                break;
        }
    }

    jack_client_close(client);

    return true;
}

unsigned int Jack::samplerate() 
{
    if (!is_running())
        THROW_ERROR("JACK server not running, cannot compare sample rates.");

    jack_client_t *client;
    jack_status_t status;
    client = jack_client_open ("AudioJackSource", JackNoStartServer, &status);
    jack_nframes_t jackRate = jack_get_sample_rate(client);
    jack_client_close(client);

    return jackRate;
}

