
/*
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
 */

/** \file
 *
 *
 *
 *
 *
 */

#include <iostream>
#include <string.h>

#include <lo/lo.h>
#include "oscMessage.h"

OscMessage::OscMessage(const char *p, const char *t, lo_arg ** v, int c, void *d)
    : path_(p), types_(t), args_(), argc_(c), data_(d)
{
    for (int i = 0; i < c; i++)
        args_.push_back(LoArg(t, i, v[i]));
}


OscMessage::OscMessage(const OscMessage& in)
    : path_(in.path_), types_(in.types_), args_(in.args_), argc_(in.argc_), data_(in.data_)
{
    // empty
}


OscMessage& OscMessage::operator=(const OscMessage& in)
{
    if (this == &in)
        return *this; // gracefully handle self-assignment

    path_ = in.path_;
    types_ = in.types_;
    args_ = in.args_;
    argc_ = in.argc_;
    data_ = in.data_;

    return *this;
}


lo_message *OscMessage::init_msg(lo_message *msg)
{
    for (OscArgs::iterator it = args_.begin(); it != args_.end(); ++it)
    {
        switch ((char) it->type_)
        {
            case 's':
            {
                lo_message_add_string(msg, it->s_.c_str());
                break;
            }
            case 'i':
            {
                lo_message_add_int32(msg, it->i_);
                break;
            }
        }
    }

    return msg;
}


void OscMessage::print()
{
    OscArgs::iterator iter;
    for (iter = args_.begin(); iter != args_.end(); ++iter)
        iter->print();

    std::cout << std::endl;
}


void OscMessage::LoArg::print() const
{
    switch ((char) type_)
    {
        case 's':
            std::cout << s_ << " ";
            break;
        case 'i':
            std::cout << i_ << " ";
            break;
    }
}


bool OscMessage::LoArg::equals(std::string str)
{
    if ((char) type_ == 's')
        return s_ == str;
    return false;
}


bool OscMessage::LoArg::equals(int val)
{
    if ((char) type_ == 'i')
        return i_ == val;
    return false;
}


OscMessage::LoArg::LoArg(const char *pchar, int index, lo_arg * a)
    : type_(static_cast<lo_type>(pchar[index])), i_(0), s_()
{
    switch ((char) type_)
    {
        case 's':
        {
            s_ = static_cast < char *>(&(a->s));
            break;
        }
        case 'i':
        {
            i_ = static_cast < int >(a->i);
            break;
        }
    }
}


