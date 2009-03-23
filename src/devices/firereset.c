// Sropulpof
// Copyright (C) 2008 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Sropulpof is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Sropulpof.  If not, see <http://www.gnu.org/licenses/>.

/**
 * Raw1394 (Firewire on GNU/Linux) bus reset.
 *
 * to compile : 
 * gcc -Wall -lraw1394 -o resetbus resetbus.c
 */
//#include <unistd.h> // usleep
#include <stdio.h> // printf
#include <libraw1394/raw1394.h>

static int bus_reset_handler(struct raw1394_handle *handle, unsigned int gen)
{
    printf("SUCCESS !!!!!!!!!\n");
    printf("Bus reset occurred.\nnew generation number: %d, %d nodes on the bus, local ID: %d\a\n",
        gen, 
        raw1394_get_nodecount(handle), 
        raw1394_get_local_id(handle) & 0x3f);
    raw1394_update_generation(handle, gen);
    return 0;
}

int main(int argc, char **argv)
{
    raw1394handle_t handle;
    int result;
    // int type = RAW1394_SHORT_RESET;
    int type = RAW1394_LONG_RESET;
    int adapter_number = 0; // see firecontrol/commander.c
    
    handle = raw1394_new_handle();
    if (handle == 0)
    {
        printf("could not get handle to firewire bus\n");
        printf("Is raw1394 module loaded ?\nIs your user in the 'disk' group ?");
        return 1;
    }
    printf("current generation number (driver): %d\n", raw1394_get_generation(handle));
    // (void)
    raw1394_set_bus_reset_handler(handle, bus_reset_handler);
    fprintf(stdout,"using adapter %d\n", adapter_number);
    if (raw1394_set_port(handle, adapter_number) < 0) 
    {
        printf("Could not set port");
        return 1;
    }
    result = raw1394_reset_bus_new(handle, type);
    if (result == -1) 
    {
        printf("Error trying to reset bus.\n");
    }
    else
    {
        printf("Trying to reset bus did not result in an error.\nResult = %d\n", result);
        //printf("(0 means success)\n");
    }
    raw1394_loop_iterate(handle);
    //usleep(1000);
    raw1394_destroy_handle(handle);
    
    return 0;
}

