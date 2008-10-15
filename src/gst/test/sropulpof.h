
// sropulpof.h
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
// // You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _SROPULPOF_H_
#define _SROPULPOF_H_

class Demo {
    public:
        Demo(short pid) : pid_(pid) {}
        virtual short run() = 0;
        virtual ~Demo(){};
    protected:
        short pid_;
        const static short NUM_CHANNELS;
};

class Sro : public Demo
{
    public:
        Sro(short pid, long txPort, long rxPort, const char *localIp, const char *remoteIp);
        ~Sro(){};
        short run();
        static const char *usage() { return "Usage: sro <0/1> txPort rxPort localIp remoteIp"; }
    private:
        long txPort_;
        long rxPort_;
        const char *localIp_;
        const char *remoteIp_;
        Sro(const Sro&);     //No Copy Constructor
        Sro& operator=(const Sro&);     //No Assignment Operator
};

class Pul : public Demo
{
    public:
        Pul(short pid);
        ~Pul(){};
        short run();
        static const char *usage() { return "Usage: pul <0/1>"; }
};


class Pof : public Demo
{
    public:
        Pof(short pid);
        ~Pof(){};
        short run();
        static const char *usage() { return "Usage: pof <0/1>"; }
};

#endif // _SROPULPOF_H_

