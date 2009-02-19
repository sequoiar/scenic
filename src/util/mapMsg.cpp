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

#include "mapMsg.h"
static msg::Subscriber* pf = 0;

msg::Subscriber::Subscriber()
{
    pf = this;
}

msg::Subscriber::~Subscriber()
{
    pf = 0;
}

bool msg::post(MapMsg& msg)
{
    if(pf)
    {
        (*pf)(msg);
        return true;
    }
    return false;
}

StrIntFloat::StrIntFloat()
    : type_('n'), s_(), i_(0), f_(0.0),e_(),F_(),key_(){}

#define T_EXPAND(x) (x == 'f'?"double":x == 's'?"string":x == 'i'?"interger":x == 'F'?"vector<double>":"unknown")

#define TYPE_CHECKMSG(gtype,xtype) \
    const char* t = #gtype; \
    char tt = t[0];\
    if(type_ == 'n') QUIET_THROW("Parameter " << key_ << " missing."); \
    else \
        if(type_ != tt) \
        QUIET_THROW("Parameter " << key_ << " should be " << #xtype << " not " << T_EXPAND(type_) << "." );\
    return gtype   

bool StrIntFloat::get(std::string& s) const
{
    if(type_ != 's')
        return false;
    s = s_;
    return true;
}

bool StrIntFloat::empty() const
{
    return type_ == 'n';
}
char StrIntFloat::get_type() const { return type_;}
std::string StrIntFloat::c_str()const
{ 
    TYPE_CHECKMSG(s_,string);
}

StrIntFloat::operator std::string ()const
{   
    TYPE_CHECKMSG(s_,string);
}

StrIntFloat::operator std::vector<double> () const
{
    TYPE_CHECKMSG(F_,vector<double>);
}

StrIntFloat::operator int ()const
{
    TYPE_CHECKMSG(i_,integer);
}

StrIntFloat::operator double ()const
{
    TYPE_CHECKMSG(f_,float);
}

StrIntFloat& StrIntFloat::operator=(const std::string& in){
    type_ = 's'; s_ = in; 
    return *this;
}

StrIntFloat& StrIntFloat::operator=(const int& in){
    type_ = 'i'; i_ = in; 
    return *this;
}

StrIntFloat& StrIntFloat::operator=(const Except& in){
    type_ = 'e'; e_ = in; 
    return *this;
}

StrIntFloat& StrIntFloat::operator=(const double& in){
    type_ = 'f'; f_ = in; 
    return *this;
}

StrIntFloat& StrIntFloat::operator=(const std::vector<double>& in){
    type_ = 'F'; F_ = in;
    return *this;
}

StrIntFloat::StrIntFloat(const StrIntFloat& sif_)
    : type_(sif_.type_), s_(sif_.s_), i_(sif_.i_), f_(sif_.f_),e_(sif_.e_),F_(sif_.F_),key_(sif_.key_){}
StrIntFloat& StrIntFloat::operator=(const StrIntFloat& in)
{
    if(this == &in)
        return *this;
    type_ = in.type_; s_ = in.s_; i_ = in.i_; f_ = in.f_; e_ = in.e_; F_ = in.F_; key_ = in.key_;
    return *this;
}


