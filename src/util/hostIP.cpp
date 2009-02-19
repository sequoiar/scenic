/* hostIP.cpp
 * Copyright (C) 2009 Société des arts technologiques (SAT)
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

#include "hostIP.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

const char *get_host_ip()
{
    int i;
    char *ip = 0;
    int s = socket (PF_INET, SOCK_STREAM, 0);
    if(s == -1)
        return ip;

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
