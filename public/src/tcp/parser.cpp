/* parser.cpp
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
 *
 *      Command parser functions used in ipcp protocol
 *
 */

#include <sstream>
#include <vector>
#include "parser.h"
#include "logWriter.h"
#include "mapMsg.h"

using namespace Parser;

typedef std::string::size_type POS;


std::string Parser::strEsq(const std::string& str)
{
    std::string out;

    for(unsigned int pos=0; 
        pos < str.size(); ++pos)                //for each char in string
    {
        char c = str[pos];                      //copy current character
        if(c == '\\')                           //if backslash found
            out.append("\\\\");                 //escape it with backslash
        else if(c == '\"')                      //if quotation mark
            out.append("\\\"");                 //escape it with backslash
        else
            out.append(1, c);                   //otherwise pass it through
    }

    return out;                                 //return copy char to output
}


static std::string strUnEsq(const std::string& str)
{
    std::string out;

    //for each char in string
    for(unsigned int pos=0; pos < str.size(); ++pos)
    {
        char c = str[pos];                          //copy first character
        char c2 = str[pos+1];                       //copy second character
        if(c == '\\')                               //if first character is backslash
        {
            if(c2 == '\\')                          //and second character is
            {                                       //also backslash
                out.append("\\");                   //add one backslash
                ++pos;                              //increment pos
            }
        }
        else
            out.append(1, c);                       //otherwise copy char to output
    }

    return out;
}


/// returns the position of the trailing quote in a string
/// ignores escaped version
static POS get_end_of_quoted_string(const std::string& str)
{
    //return error if string doesn't start with "
    if(str[0] != '\"'){
        LOG_WARNING("String must start with \".");
        return 0;
    }
    //for each char in string
    for(unsigned int pos=1; pos < str.size(); ++pos)
    {
        if(str[pos] == '\"')                       //if char is " and if
            if(str[pos-1] != '\\')                 //previous char is not escape char
                return pos;                        //return position following "
    }

    THROW_ERROR("String has no terminating \".");
    return 0;
}


bool Parser::stringify(MapMsg& cmd_map, std::string& rstr)
{
    std::stringstream sstr;
    rstr.clear(); 
    //locate "command" and output value to str
    sstr << std::string(cmd_map["command"]) << ":";
    const std::pair<const std::string,StrIntFloat>* it;

    //for each pair in the map
    for(it = cmd_map.begin(); it != 0; it = cmd_map.next())
    {   
        //Insure it's not the command
        if(it->first != "command")    
        {
            //Output the key to the str
            sstr << " " << it->first << "=";
            //Output the value to the str
            switch(it->second.type()) 
            {
                //the value is a string, delimit with "
                case 's':
                    //Escape the string contents 
                    sstr << "\"" << strEsq(it->second) << "\""; 
                    break;
                
                //the value is integer, generate string from int
                case 'i': 
                    sstr << int(it->second);
                    break;
                    
                //the value is a float generate string from float
                case 'f': 
                    sstr << double(it->second);
                    break;

                case 'F':
                    {
                        const std::vector<double> &v = it->second;
                        sstr << v[0];
                        //TODO: parse and tokenize vectors
                    }
                    break;

                default:
                    THROW_ERROR("Command " << it->first 
                        << " has unknown type " << it->second.type());
            }       
        }
    }
    rstr = sstr.str();
    return true;
}


static void erase_to_end_of_whitespace(std::string &in)          
{
    POS pos = in.find_first_of(' ');
    in.erase(0,pos);
    pos = in.find_first_not_of(' ');
    if(pos != std::string::npos)
        in.erase(0,pos);
}

static bool contains_only(const std::string& in, char c)
{
    return (in.find_first_not_of(c) == std::string::npos);
}


static bool contains(const std::string& in, char c)
{
    return (in.find_first_of(c) != std::string::npos);
}


bool Parser::tokenize(const std::string& str, MapMsg &cmd_map)
{
    std::string::size_type tok_end;
    std::string lstr = str;                                     //copy in str to local string
    cmd_map.clear();                                            //clear output map

    tok_end = lstr.find_first_of(':');                          //search for ":"
    if(tok_end == lstr.size())                                  //if : not found return error
        THROW_ERROR("No command found.");

    cmd_map["command"] = lstr.substr(0, tok_end);               //insert command into map
    erase_to_end_of_whitespace(lstr);                           //set lstring beyond command
    if(lstr.empty())
        return true;

    //loop until break or return
    for(;;)
    {
        tok_end = lstr.find_first_of('=');                      //search of "="
        if(tok_end == std::string::npos)                        //if not found no more
            break;                                              //key=value pairs so break loop
        std::string key_str = lstr.substr(0,tok_end);
        ++tok_end;
        lstr.erase(0,tok_end);                                  //lstr points to value
        if(lstr[0] == '\"')                                     //if value looks like a string
        {
            POS end_quote = get_end_of_quoted_string(lstr);     //find end of "
            if(end_quote == 0)                                  //if value not string
                THROW_ERROR("No end of quote found.");

            std::string quote = lstr.substr(1, end_quote-1);        //strip begin and end quotes
            quote = strUnEsq(quote);                                //clean any escape backslashes

            cmd_map[key_str] = quote;                               //insert key,value into map
            lstr.erase(0,end_quote);
            erase_to_end_of_whitespace(lstr);                       //set lstring beyond command
        }
        else{
            std::vector<double> vd;
            std::vector<int> vi;
            for(;;)
            {
                POS pos = lstr.find_first_of(" =");
                if(lstr[pos] == '=')
                    break;
                std::stringstream stream(lstr.substr(0,pos));       //string of value
                lstr.erase(0,pos);
                erase_to_end_of_whitespace(lstr);                   //set lstring beyond command
                if(contains(stream.str(),'='))
                    break;
                    
                //if value does not contain . it is an int
                if(!contains(stream.str(),'.'))
                {
                    int temp_i;
                    stream >> temp_i;                               //convert str to int
                    vi.push_back(temp_i);                           //stor val
                    LOG_DEBUG(key_str << " " << temp_i);
                }
                else    //value contains . thus it is a float
                {                                                   
                    float temp_f;
                    stream >> temp_f;                               //convert str to float
                    vd.push_back(temp_f);                           //stor val
                }
                if(contains_only(lstr,' '))                         //Only whitespace remains
                    break;
            }
            if(vd.size() > 0 && vi.size() > 0)
                THROW_ERROR("Cannot mix types in arrays");

            if(vd.size() == 1)
                cmd_map[key_str] = vd[0];
            else if (vd.size() > 1)
                cmd_map[key_str] = vd[0];
                //TODO replace above with ->  cmd_map[key_str] = vd;

            if(vi.size() == 1)
                cmd_map[key_str] = vi[0];
            else if (vi.size() > 1)
                cmd_map[key_str] = vi[0];
                //TODO replace above with ->  cmd_map[key_str] = vi;
        }
        if(contains_only(lstr,' '))                         //Only whitespace remains
            break;                                              
            
    }

    return true;
}


