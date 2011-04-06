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

#include <gst/gst.h>
#include <string>
#include <iostream>
#include <tr1/memory>
#include "caps/caps_server.h"
#include "caps/caps_client.h"

int main(int argc, char **argv)
{
    gst_init(&argc, &argv);
    int result;
    GstCaps *sendCaps;
    GstCaps *receivedCaps;
    sendCaps = gst_caps_new_simple ("video/x-raw-yuv",
            "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),
            "framerate", GST_TYPE_FRACTION, 25, 1,
            "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
            "width", G_TYPE_INT, 320,
            "height", G_TYPE_INT, 240,
            NULL);
    const std::string testCapsStr(gst_caps_to_string(sendCaps));
    std::tr1::shared_ptr<CapsServer> capsServer(new TcpCapsServer(10000, testCapsStr));
    CapsClient capsClient("127.0.0.1", "10000");
    const std::string receivedCapsStr(capsClient.getCaps());
    receivedCaps = gst_caps_from_string(receivedCapsStr.c_str());

    if (gst_caps_is_equal(sendCaps, receivedCaps))
    {
        std::cerr << "Received caps match sent caps" << std::endl;
        result = 0;
    }
    else
    {
        std::cerr << "Received caps do not match sent caps" << std::endl;
        result = 1;
    }
    gst_caps_unref(sendCaps);
    gst_caps_unref(receivedCaps);

    return result;
}
