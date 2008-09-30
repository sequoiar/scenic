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
 *      MapMsg  key/value map where value is a string, a float, an int, a vector
 *
 *
 */

#include "mapMsg.h"
static MsgFunctor* pf = 0;

bool MSG::post(MapMsg& msg)
{
    if(pf)
    {
        MsgFunctor& func(*pf);
        func(msg);
        return true;
    }
    return false;
}

void MSG::register_cb(MsgFunctor* f)
{
    pf = f;
}
void MSG::unregister_cb()
{
    pf = 0;
}


StrIntFloat::StrIntFloat(std::string s)
    : type_('s'), s_(s), i_(0), f_(0.0), e_(),F_(){}
StrIntFloat::StrIntFloat(int i)
    : type_('i'), s_(), i_(i), f_(0.0), e_(),F_(){}
StrIntFloat::StrIntFloat(double f)
    : type_('f'), s_(), i_(0), f_(f), e_(),F_(){}
StrIntFloat::StrIntFloat(Except e)
    : type_('e'), s_(), i_(0), f_(0.0),e_(e),F_(){}
StrIntFloat::StrIntFloat()
    : type_('n'), s_(), i_(0), f_(0.0),e_(),F_(){}
StrIntFloat::StrIntFloat(std::vector<double> F)
    : type_('F'), s_(), i_(0), f_(0.0),e_(),F_(F){}

char StrIntFloat::type() const { return type_;}
std::string StrIntFloat::c_str()const
{       
    if(type_ != 's')
        THROW_ERROR("Type is " << type_  << " not string");                     
    return s_;
}

StrIntFloat::operator std::string ()const
{   
    return c_str();
}
bool StrIntFloat::get(std::string& s) const
{
    if(type_ != 's')
        return false;
    s = s_;
    return true;
}

StrIntFloat::operator std::vector<double> () const
{
    return F_;
}

StrIntFloat::operator int ()const
{
    if(type_ != 'i')
        THROW_ERROR("Type is " << type_  << " not integer");                     
    return i_;
}
bool StrIntFloat::get(int& i) const
{
    if(type_ != 'i')
        return false;
    i = i_;
    return true;
}

StrIntFloat::operator double ()const
{
    if(type_ != 'f')
        THROW_ERROR("Type is " << type_  << " not float");                     
    return f_;
}

bool StrIntFloat::get(double& f) const
{
    if(type_ != 'f')
        return false;
    f = f_;
    return true;
}

bool StrIntFloat::get(Except& e) const
{
    if(type_ != 'e')
        return false;
    e = e_;
    return true;
}

StrIntFloat& StrIntFloat::operator=(const std::string& in){
    type_ = 's'; s_ = in; 
    return *this;
}

StrIntFloat& StrIntFloat::operator=(int in){
    type_ = 'i'; i_ = in; 
    return *this;
}

StrIntFloat& StrIntFloat::operator=(double in){
    type_ = 'f'; f_ = in; 
    return *this;
}

StrIntFloat::StrIntFloat(const StrIntFloat& sif_)
    : type_(sif_.type_), s_(sif_.s_), i_(sif_.i_), f_(sif_.f_),e_(sif_.e_),F_(sif_.F_){}
StrIntFloat& StrIntFloat::operator=(const StrIntFloat& in)
{
    if(this == &in)
        return *this;
    type_ = in.type_; s_ = in.s_; i_ = in.i_; f_ = in.f_; e_ = in.e_; F_ = in.F_;
    return *this;
}


