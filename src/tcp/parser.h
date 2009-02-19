/* parser.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
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

#ifndef __PARSER_H__
#define __PARSER_H__

#include "mapMsg.h"
/** Parser builds MapMsg from string and stringify from MsgMap
 *  - Command parser functions used in ipcp protocol */
namespace Parser
{
    /// builds a string of command: key=value pairs 
    bool stringify(MapMsg& cmd_map, std::string& str);

    /** fills a map of key=value pairs with a special key "command" from a string
     * of type specified above */
    bool tokenize(const std::string& str, MapMsg& cmd_map);
}
#endif

