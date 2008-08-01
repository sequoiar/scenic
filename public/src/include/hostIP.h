
// hostIP.h
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
 *      Just the License GPL 3+
 *
 *      Detailed description here.
 *      Continues here.
 *      And more.
 *      And more.
 */

#ifndef _HOST_IP_H_
#define _HOST_IP_H_


#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

static
const char *get_host_ip()
{
    int i;
    char *ip;
    int s = socket (PF_INET, SOCK_STREAM, 0);

    for (i = 1;; i++)
    {
        struct ifreq ifr;
        struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;

        ifr.ifr_ifindex = i;
        if (ioctl (s, SIOCGIFNAME, &ifr) < 0)
            break;
        /* now ifr.ifr_name is set */
        if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
            continue;
        ip = inet_ntoa (sin->sin_addr);         // seems to be thread safe
                                                // but not reentrant
                                                // under libc6 2+
    }

    close (s);
    return ip;
}


#endif // _HOST_IP_H_
