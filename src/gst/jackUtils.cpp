/* jackUtils.cpp
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
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include <jack/jack.h>
#include "jackUtils.h"
#include "pipeline.h"

bool Jack::is_running()
{
    jack_client_t *client;
    jack_status_t status;

    /* Open a client connection to the JACK server.  Starting a
     * new server only to see if it's running is pointless, so we
     * specify JackNoStartServer. */

    client = jack_client_open ("AudioJackSource", JackNoStartServer, &status);

    if (client == NULL and (status & JackServerFailed))
        return false;
    if (client == NULL) 
    {
        switch (status)
        {
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
            case JackFailure:   
                THROW_ERROR("Overall operation failed.");
                break;
            default:
                THROW_ERROR("Overall operation failed for unknown reason.");
                break;
        }
    }

    jack_client_close(client);

    return true;
}


jack_nframes_t Jack::samplerate() 
{
    if (!is_running())
        THROW_ERROR("JACK server not running, cannot get sample rate.");

    jack_client_t *client;
    jack_status_t status;
    client = jack_client_open ("AudioJackSource", JackNoStartServer, &status);
    jack_nframes_t jackRate = jack_get_sample_rate(client);
    jack_client_close(client);

    return jackRate;
}


/// Check that jack is running and is at the right sample rate
void Jack::assertReady(Pipeline &pipeline)
{
    if (!Jack::is_running())
        THROW_CRITICAL("Jack is not running");

    if (pipeline.actualSampleRate() != samplerate())
    {
        LOG_WARNING("Jack's sample rate of " << samplerate()
                << " does not match pipeline sample rate " << pipeline.actualSampleRate());
        pipeline.updateSampleRate(samplerate());
    }
}


unsigned int Jack::framesPerPeriod() 
{
    if (!is_running())
        THROW_ERROR("JACK server not running, cannot compare sample rates.");

    jack_client_t *client;
    jack_status_t status;
    client = jack_client_open ("AudioJackSource", JackNoStartServer, &status);
    jack_nframes_t framesPerPeriod = jack_get_buffer_size(client);
    jack_client_close(client);

    return framesPerPeriod;
}

// DEPENDS ON sample rate and frames per period, doesn't appear to depend on periods per buffer
unsigned long long Jack::minBufferTime()
{
    const unsigned long long USECS_PER_SEC = 1000000LL;
    return  ((framesPerPeriod() / static_cast<float>(samplerate())) * USECS_PER_SEC); 
}

unsigned long long Jack::safeBufferTime()
{
    return minBufferTime();
}

