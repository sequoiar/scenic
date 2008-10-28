
// raw1394util.cpp
// Based on raw1394util.c, part of dvgrab available at <http://www.kinodv.org/>
// Copyright 2008 Koya Charles & Tristan Matthews //
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

#include <cerrno>
#include <cstring>
#include "config.h"
#ifdef CONFIG_IEEE1394
#include <libavc1394/avc1394.h>
#include <libavc1394/rom1394.h>
#endif
#include "raw1394Util.h"
#include "logWriter.h"

#define MOTDCT_SPEC_ID    0x00005068


/** Open the raw1394 device and get a handle.
 *  
 * \return number of ports found
 */
#ifdef CONFIG_IEEE1394
int raw1394_get_num_ports()
{
	int n_ports;
	struct raw1394_portinfo pinf[ 16 ];
	raw1394handle_t handle;

	/* get a raw1394 handle */
	if (!(handle = raw1394_new_handle()))
		THROW_ERROR("raw1394 - failed to get handle: " << strerror(errno));

	n_ports = raw1394_get_port_info(handle, pinf, 16);
	raw1394_destroy_handle(handle);

	if (n_ports  < 0)
		THROW_ERROR("raw1394 - failed to get port info: " << strerror(errno));

	return n_ports;
}


/** Open the raw1394 device and get a handle.
 *  
 * \param port A 0-based number indicating which host adapter to use.
 * \return a raw1394 handle.
 */

#ifdef RAW1394_V_0_8
	raw1394handle_t (* const rawHandle)(void) = raw1394_get_handle;
#else
	raw1394handle_t (* const rawHandle)(void) = raw1394_new_handle;
#endif

raw1394handle_t raw1394_open(int port)
{
	struct raw1394_portinfo pinf[ 16 ];
	/* get a raw1394 handle */
	raw1394handle_t handle = rawHandle();


	if (!handle)
		THROW_ERROR("raw1394 - failed to get handle: " << strerror(errno) );

	if (raw1394_get_port_info( handle, pinf, 16 ) < 0 )
	{
		raw1394_destroy_handle(handle);
		THROW_ERROR("raw1394 - failed to get port info: " <<  strerror(errno));
	}

	/* tell raw1394 which host adapter to use */
	if (raw1394_set_port(handle, port) < 0)
	{
		raw1394_destroy_handle(handle);
		THROW_ERROR("raw1394 - failed to set set port: " <<  strerror(errno) );
	}

	return handle;
}


int discoverAVC(int* port, octlet_t* guid)
{
	rom1394_directory rom_dir;
	raw1394handle_t handle;
	int device = -1;
	int i, j = 0;
	int m = raw1394_get_num_ports();

	if (*port >= 0)
	{
		/* search on explicit port */
		j = *port;
		m = *port + 1;
	}

	for (; j < m && device == -1; j++)
	{
		handle = raw1394_open(j);
		for (i = 0; i < raw1394_get_nodecount(handle); ++i)
		{
			if (*guid > 1)
			{
				/* select explicitly by GUID */
				if (*guid == rom1394_get_guid(handle, i))
				{
					device = i;
					*port = j;
					break;
				}
			}
			else
			{
				/* select first AV/C Tape Reccorder Player node */
				if (rom1394_get_directory(handle, i, &rom_dir) < 0)
				{
					rom1394_free_directory(&rom_dir);
					THROW_ERROR("error reading config rom directory for node " << i);
                    //TODO: note: never gets to continue
					continue;
				}
				if (((rom1394_get_node_type(&rom_dir) == ROM1394_NODE_TYPE_AVC) &&
				         avc1394_check_subunit_type(handle, i, AVC1394_SUBUNIT_TYPE_VCR)) ||
				       (rom_dir.unit_spec_id == MOTDCT_SPEC_ID))
				{
					rom1394_free_directory(&rom_dir);
					octlet_t my_guid, *pguid = (*guid == 1)? guid : &my_guid;
					*pguid = rom1394_get_guid( handle, i );
					LOG_DEBUG("Found AV/C device with GUID 0x" << 
						(quadlet_t) (*pguid>>32) << (quadlet_t) (*pguid & 0xffffffff) << std::endl);
					device = i;
					*port = j;
					break;
				}
				rom1394_free_directory(&rom_dir);
			}
		}
	    raw1394_destroy_handle(handle);
	}

	return device;
}

	//static int raw1394_get_num_ports();
	//static raw1394handle_t raw1394_open( int port );
	//static int discoverAVC( int * port, octlet_t* guid );

bool Raw1394::cameraIsReady() 
{
    raw1394handle_t handle;
    int port, device;
    octlet_t guid = 0;
    port = device = -1;

    if (!(handle = raw1394_new_handle()))
        THROW_ERROR("raw1394 cannot get handle");

    raw1394_destroy_handle(handle);

    if (discoverAVC(&port, &guid) == -1)
        THROW_ERROR("Dv source is not ready");

    return true;
}
#endif
