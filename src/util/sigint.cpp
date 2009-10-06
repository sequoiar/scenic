
/* sigint.cpp
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

#include <signal.h>
#include "sigint.h"
#include "logWriter.h"
#include "msgThread.h"

static bool signal_flag = false;

bool signal_handlers::signalFlag()
{
    return signal_flag;
}

static void signalHandler(int sig, siginfo_t* /* si*/, void* /* unused*/)
{
    LOG_INFO("Got signal " << sig << ", going down!");
    signal_flag = true;
    MsgThread::broadcastQuit();
}


void signal_handlers::setHandlers()
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signalHandler;
    const int NUM_SIGNALS = 5; 
    const int signals[NUM_SIGNALS]  = {SIGHUP, SIGINT, SIGQUIT, SIGABRT, SIGTERM};
    for (int sig = 0; sig != NUM_SIGNALS; ++sig)
        if (sigaction(signals[sig], &sa, NULL) == -1)
            THROW_ERROR("Cannot register signal " << signals[sig] 
                    << " handler");
}

