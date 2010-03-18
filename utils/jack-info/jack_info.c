/*---------------------------------------------------------------------
 jack_info.c

 Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 http://www.sat.qc.ca
 All rights reserved.

 This file is part of [propulse]ART.

 [propulse]ART is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 [propulse]ART is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
---------------------------------------------------------------------*/

#include <stdio.h>
#include <jack/jack.h>

int main()
{
    int i;
    const char **ports;
    jack_nframes_t period;
    jack_nframes_t rate;

    jack_client_t *client;
    jack_status_t status;
    client = jack_client_open("jack-info", JackNoStartServer, &status);
	if (client == NULL) 
    {
		if (status & JackServerFailed) 
			fprintf(stderr, "JACK server not running\n");
		else 
			fprintf(stderr, "jack_client_open() failed, "
				 "status = 0x%2.0x\n", status);
		return 1;
	}

    period = jack_get_buffer_size(client);
    rate = jack_get_sample_rate(client);

	ports = jack_get_ports(client, NULL, NULL, 0);
   
	for (i = 0; ports[i] != 0; ++i) 
    {
		jack_port_t *port = jack_port_by_name(client, ports[i]);
        if (port != 0) 
        {
            int flags = jack_port_flags (port);
            if (flags & JackPortIsPhysical)
            {
		printf ("%s\n", ports[i]);
		/* show_port_latency */
				printf("	port latency = %f ms\n",
                        (jack_port_get_latency(port) / (double) rate) * 1000);
		/* show_total_latency */
				printf ("	total latency = %f ms\n",
					(jack_port_get_total_latency(client, port) / (double) rate) * 1000);
            }
		}
    }
    jack_client_close(client);

    printf(
    "buffer-size:   %d frames\n"
    "samplerate:    %d\n",
    period,
    rate);
    return 0;
}
