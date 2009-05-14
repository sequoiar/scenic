/* MapMsg.h
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

#ifndef __MAP_MSG_H__
#define __MAP_MSG_H__

#include "util.h"
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

class MapMsg;
/// container class for variable types used by MapMsg 
class StrIntFloat
{
friend std::ostream& operator<< (std::ostream& os, const StrIntFloat&);
friend class MapMsg;
    public:
        StrIntFloat()
            : type_('n'), s_(), i_(0), f_(0.0),e_(),F_(),key_(){}
        char get_type() const; 
        bool empty() const;
        Except except()const { return e_;}

        operator std::string () const;
        operator std::vector<double> () const;
        operator int ()const;
        operator double ()const;
        operator bool ()const;

        bool operator==(const StrIntFloat& sif);
        bool operator==(const std::string& in);
        StrIntFloat& operator=(const std::string& in);
        StrIntFloat& operator=(const int& in);
        StrIntFloat& operator=(const Except& in);
        StrIntFloat& operator=(const double& in);
        StrIntFloat& operator=(const std::vector<double>& in);

        StrIntFloat(const StrIntFloat& sif);
        StrIntFloat& operator=(const StrIntFloat& in);
    private:
        char type_;
        std::string s_;
        int i_;
        double f_;
        Except e_;
        std::vector<double> F_;
        std::string key_; //HACK
};

std::ostream& operator<< (std::ostream& os, const StrIntFloat&);


/// key/value map where value is a string, a float, an int, a vector StrIntFloat
class MapMsg
{
public:
    typedef std::map<std::string, StrIntFloat> MapMsg_;
    typedef const std::pair<const std::string,StrIntFloat>* Item;
    MapMsg():map_(),it_(){}
    MapMsg(std::string command):map_(),it_(){ cmd() = command;}
    StrIntFloat &cmd() { return (*this)["command"]; }
    StrIntFloat &operator[](const std::string& str);
    void tokenize(const std::string& str) { tokenize(str,*this); }
    bool stringify(std::string& str) const { return stringify(*this,str); }
    void clear(){map_.clear();}

/** Used by code that needs to post messages but does not use 
 * a MsgThread class (gst/audioLevel.cpp) send a MapMsg to Subscriber */

    bool post();

    /// MapMsg will go to most recent registered  
    class Subscriber
    {
        public:
            Subscriber();
            virtual void operator()(MapMsg&){}
            virtual ~Subscriber();
    };
private:
    friend std::ostream& operator<< (std::ostream& os, const MapMsg&);
    friend Item GetBegin(MapMsg& m);
    friend Item GetNext(MapMsg& m);

    MapMsg_ map_;
    MapMsg_::const_iterator it_;
    static void tokenize(const std::string& str, MapMsg &cmd_map);
    static bool stringify(const MapMsg& cmd_map, std::string& rstr);
    Item begin();
    Item next();

};

std::ostream& operator<< (std::ostream& os, const MapMsg&);

#endif

