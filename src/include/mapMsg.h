// MapMsg.h
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
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//


/** \file 
 *      MapMsg typedef of key/value map where value is a string, a float, an int or exception
 *
 */


#ifndef __MAP_MSG_H__
#define __MAP_MSG_H__

#include "logWriter.h"
#include <string>
#include <map>
#include <vector>
#include <sstream>


class StrIntFloat
{
    char type_;
    std::string s_;
    int i_;
    double f_;
    Except e_;
    std::vector<double> F_;
    public:
        StrIntFloat();
        std::string k_;
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
};

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
        sif.k_ = str; 
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
#if 0 
template< class T >
void msg_get_(MapMsg& msg,const std::string& key, const std::string& val_type, T& t)
{
    if(msg[key].type() == 'n')                                          
    { 
        std::ostringstream err; 
        err << "key:" << key << " missing."; 
        THROW_ERROR(err.str()); 
    }
    if(!msg[key].get(t)) 
    { 
        std::ostringstream err; 
        err << "Expected type " << val_type << " does not match "
            << msg[key].type() << " provided by user."; 
        THROW_ERROR(err.str()); 
    } 
}
#endif
class MsgFunctor
{
    public:
        virtual void operator()(MapMsg&){}
        virtual ~MsgFunctor(){}
};

namespace MSG
{
bool post(MapMsg& msg);
void register_cb(MsgFunctor* f);
void unregister_cb();
}


#endif

