/*
 * Copyright (C) 2008-2011 Société des arts technologiques (SAT)
 * Tristan Matthews
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <string>
#include <iostream>
#include <tr1/memory>
#include "caps/caps_server.h"
#include "caps/caps_client.h"

int main()
{
    const std::string testCaps("THESE ARE SOME CAPS");
    std::tr1::shared_ptr<CapsServer> capsServer(new TcpCapsServer(10000, testCaps));
    CapsClient capsClient("127.0.0.1", "10000");
    std::string receivedCaps(capsClient.getCaps());
    if (receivedCaps == testCaps)
    {
        std::cerr << "Received caps match expected caps" << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "Received caps do not match expected caps" << std::endl;
        return 1;
    }
}
