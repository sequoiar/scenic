/* 
 * Copyright 2008 Koya Charles & Tristan Matthews 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/** \file 
 *      Just the License LGPL 3+ 
 *
 *      Detailed description here.
 *      Continues here.
 *      And more.
 *      And more.
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <map>

std::string strEsq(const std::string& str);
std::string strUnEsq(const std::string& str);
int get_end_of_quoted_string(const std::string& str);

bool tokenize(const std::string& str, std::map<std::string,std::string> &cmd_map) ;

#endif

