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
 *      MapMsg typedef of key/value map where value is a string, a float or an int
 *
 */


#ifndef __MAP_MSG_H__
#define __MAP_MSG_H__

#include "logWriter.h"
#include <string>
#include <map>
#include <sstream>



#define GET(msg,key,val_type,val)                                           \
    val_type val;                                                           \
    msg_get_(msg,key,#val_type,val)


class StrIntFloat
{
    char type_;
    std::string s_;
    int i_;
    float f_;
    Except e_;
    public:
        StrIntFloat(std::string s)
            : type_('s'), s_(s), i_(0), f_(0.0), e_(){}
        StrIntFloat(int i)
            : type_('i'), s_(), i_(i), f_(0.0), e_(){}
        StrIntFloat(float f)
            : type_('f'), s_(), i_(0), f_(f), e_(){}
        StrIntFloat(Except e)
            : type_('e'), s_(), i_(0), f_(0.0),e_(e){}

        StrIntFloat()
            : type_('n'), s_(), i_(0), f_(0.0),e_(){}

        char type() const { return type_;}
        bool get(std::string& s) const
        {
            if(type_ != 's')
                return false;
            s = s_;
            return true;
        }


        bool get(int& i) const
        {
            if(type_ != 'i')
                return false;
            i = i_;
            return true;
        }


        bool get(float& f) const
        {
            if(type_ != 'f')
                return false;
            f = f_;
            return true;
        }

        bool get(Except& e) const
        {
            if(type_ != 'e')
                return false;
            e = e_;
            return true;
        }


        StrIntFloat(const StrIntFloat& sif_)
            : type_(sif_.type_), s_(sif_.s_), i_(sif_.i_), f_(sif_.f_),e_(sif_.e_){}
        StrIntFloat& operator=(const StrIntFloat& in)
        {
            if(this == &in)
                return *this;
            type_ = in.type_; s_ = in.s_; i_ = in.i_; f_ = in.f_; e_ = in.e_;
            return *this;
        }
};

typedef std::map<std::string, StrIntFloat> MapMsg;


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
        char t_ = msg[key].type();                                           
        err << "Expected type " << val_type << " does not match " 
            << (t_ == 'i'?"integer":t_ == 'f'?"float":"string")                      
            << " provided by user.";                                            
        THROW_ERROR(err.str());                                               
    }                                                                       
}

#endif
