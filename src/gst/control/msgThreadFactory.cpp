/* builder.cpp
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


#include "msgThreadFactory.h"
#include "tcp/tcpThread.h"
#include "gstSenderThread.h"
#include "gstReceiverThread.h"

MsgThread* MsgThreadFactory::Tcp(int port, bool log)
{
    return ( new TcpThread(port, log) );
}


MsgThread* MsgThreadFactory::Gst(bool send)
{
    if(send)
        return new GstSenderThread();
    else
        return new GstReceiverThread();
}

