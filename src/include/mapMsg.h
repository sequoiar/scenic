/* MapMsg.h
 * Copyright 2008 Koya Charles & Tristan Matthews 
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

#include "logWriter.h"
#include <string>
#include <map>
#include <vector>
#include <sstream>

/// container class for variable types used by MapMsg 
class StrIntFloat
{
    public:
        StrIntFloat();
        char get_type() const; 
        bool empty() const;
        std::string c_str()const;
        Except except()const { return e_;}

        operator std::string ()const;
        operator std::vector<double> () const;
        operator int ()const;
        operator double ()const;

        bool get(std::string& s) const;
        bool get(int& i) const;
        bool get(double& f) const;
        bool get(Except& e) const;

        StrIntFloat& operator=(const std::string& in);
        StrIntFloat& operator=(const int& in);
        StrIntFloat& operator=(const double& in);
        StrIntFloat& operator=(const Except& in);
        StrIntFloat& operator=(const std::vector<double>& in);

        StrIntFloat(const StrIntFloat& sif_);
        StrIntFloat& operator=(const StrIntFloat& in);
    private:
        char type_;
        std::string s_;
        int i_;
        double f_;
        Except e_;
        std::vector<double> F_;
    public: //HACK
        std::string key_;
};


/// key/value map where value is a string, a float, an int, a vector StrIntFloat
class MapMsg
{
    typedef std::map<std::string, StrIntFloat> MapMsg_;
    MapMsg_ map_;
    MapMsg_::const_iterator it_;
public:
    MapMsg():map_(),it_(){}
    MapMsg(std::string cmd):map_(),it_(){ map_["command"] = cmd;}

    StrIntFloat& operator[](const std::string& str)
    {    
        StrIntFloat& sif = map_[str];
        sif.key_ = str; 
        return sif;
    }
    void clear(){map_.clear();}
    const std::pair<const std::string,StrIntFloat>* begin(){it_ =map_.begin(); return &(*it_);}
    const std::pair<const std::string,StrIntFloat>* next()
    { 
        if (++it_ != map_.end())
            return &(*it_);
        else 
            return 0;
    }
};


/** Used by code that need msg posting but does not use 
 * a MsgThread class eg. gst/audioLevel.cpp */
namespace msg
{
    /// to send a MapMsg to Subscriber 
    bool post(MapMsg& msg);
    /// MapMsg will go to most recent registered  
    class Subscriber
    {
        public:
            Subscriber();
            virtual void operator()(MapMsg&){}
            virtual ~Subscriber();
    };
}


#endif

