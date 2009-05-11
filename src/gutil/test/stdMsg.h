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
//

/** \file
 *
 *
 *      Detailed description here.
 *      Continues here.
 *      And more.
 *      And more.
 */


#ifndef __STD_MSG__
#define __STD_MSG__

class StdMsg
{
    public:
        enum type { ERROR = -2, UNDEFINED = -1, NONE = 0, STD = 'm', OK = '=', QUIT = 'q',
                    INIT = 'I',
                    START = 'S', STOP = '!', SYSTEM = '*', PING = '.', STRING = 's'};

        StdMsg(unsigned short i)
            : t_(static_cast<unsigned char>(i)), msg_(){}

        StdMsg(const char *str)
            : t_(static_cast<unsigned char>(str[0])), msg_(){}


        StdMsg(char i)
            : t_(i), msg_(){}

        StdMsg(type t, std::string msg)
            : t_(t), msg_(msg){}

        StdMsg(type t)
            : t_(t), msg_(){}

        StdMsg()
            : t_(0), msg_(){}

        int get_int(){
            return static_cast<int>(t_);
        }


        char get_char(){
            return t_;
        }


        type get_type(){
            return static_cast<type>(t_);
        }


        std::string& getMsg(){return msg_;}

    private:
        unsigned char t_;

        std::string msg_;
};


#endif // __STD_MSG__

