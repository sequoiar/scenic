// jack_info.c
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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

#include <stdio.h>

int main(int argc, char *argv[])
{
    const char* backend = "alsa";
    const char* device = "hw:0";
    const char* name = "default";
    int nperiods = 2;
    int period = 1024;
    int pid = 7471;
    int rate = 44100;

    printf(
    "backend:   %s  \n"
    "device:    %s  \n"
    "name:      %s  \n"
    "nperiods:  %d  \n"
    "period:    %d  \n"
    "pid:       %d  \n"
    "rate:      %d  \n",
    backend,
    device,
    name,
    nperiods,
    period,
    pid,
    rate);
    return 0;
}
