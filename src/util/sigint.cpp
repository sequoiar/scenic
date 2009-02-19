
/* sigint.cpp
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

#include <signal.h>
#include "sigint.h"
#include "logWriter.h"

static bool signal_flag = false;

bool signalFlag()
{
    return signal_flag;
}

static void handler(int /*sig*/, siginfo_t* /* si*/, void* /* unused*/)
{
    LOG_INFO("Got SIGINT going down!");
    signal_flag = true;

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = NULL;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        THROW_ERROR("Cannot register SIGINT handler");
}


void set_handler()
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        THROW_ERROR("Cannot register SIGINT handler");
}

