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
#include <sstream>
#include "parser.h"
#include "logWriter.h"
#include "gutil/strIntFloat.h"


std::string strEsq(const std::string& str)
{
    std::string out;

    for(unsigned int pos=0; pos < str.size(); ++pos) //for each char in string
    {
        char c = str[pos];              //copy current character
        if(c == '\\')                 //if backslash found
            out.append("\\\\");      //escape it with backslash
        else if(c == '\"')            //if quotation mark
            out.append("\\\"");      //escape it with backslash
        else
            out.append(1, c);           //otherwise pass it through
    }

    return out;                         //return copy char to output
}


std::string strUnEsq(const std::string& str)
{
    std::string out;

    //for each char in string
    for(unsigned int pos=0; pos < str.size(); ++pos)
    {
        char c = str[pos];              //copy first character
        char c2 = str[pos+1];           //copy second character
        if(c == '\\')                   //if first character is backslash
        {
            if(c2 == '\\')              //and second character is
            {                           //also backslash
                out.append("\\");       //add one backslash
                ++pos;                  //increment pos
            }
        }
        else
            out.append(1, c);           //otherwise copy char to output
    }

    return out;
}


int get_end_of_quoted_string(const std::string& str)
{
    //return error if string doesn't start with "
    if(str[0] != '\"'){
        LOG_WARNING("String must start with \".");
        return 0;
    }
    //for each char in string
    for(unsigned int pos=1; pos < str.size(); ++pos)
    {
        if(str[pos] == '\"')          //if char is " and if
            if(str[pos-1] != '\\')    //previous char is not escape char
                return pos+1;           //return position following "
    }

    LOG_WARNING("String has no terminating \".");
    return 0;
}


bool stringify(std::map<std::string, StrIntFloat>& cmd_map, std::string& str)
{
    typedef std::map<std::string, StrIntFloat>::iterator iter;
    str.clear();

    //locate "command" and output it to str
    iter it = cmd_map.find(std::string("command"));
    if(it != cmd_map.end())
    {
        std::string temp;
        if(!it->second.get(temp))
            return false;
        str.append(temp);
    }
    else{
        return false;
    }
    str.append(":");

    //for each pair in the map
    for(it = cmd_map.begin();
        it != cmd_map.end(); ++it)
    {
        if(it->first.compare("command"))        //Insure it's not the command
        {
            str.append(" ");
            str.append(it->first);              //Output the key to the str
            str.append("=");
            switch(it->second.type())           //Output the value to the str
            {
                case 's':                       //If the value is a string
                {                               //delimit with "
                    std::string temp;
                    if(!it->second.get(temp))
                        return false;
                    str.append("\"");
                    temp = strEsq(temp);        //Escape the string contents
                    str.append(temp);
                    str.append("\"");
                    break;
                }
                case 'i':                       //If the value is integer
                {                               //generate string from int
                    int temp;
                    if(!it->second.get(temp))
                        return false;
                    std::stringstream sstream;
                    sstream << temp;
                    str.append(sstream.str());
                    break;
                }
                case 'f':                       //If the value is a float
                {                               //generate string from float
                    float temp;
                    if(!it->second.get(temp))
                        return false;
                    std::stringstream sstream;
                    sstream << temp;
                    str.append(sstream.str());
                    break;
                }
                default:
                    return false;
            }
        }
    }

    str.append("\r\\\n");                       //Append \r\n to conform to
    //telnet specification

    return true;
}


bool tokenize(const std::string& str, std::map<std::string, StrIntFloat> &cmd_map)
{
    unsigned int i;
    const char *cstr = str.c_str();
    std::string lstr = str;                             //copy in str to local string
    cmd_map.clear();                                    //clear output map

    i = strcspn(cstr, ":");                              //search for ":"
    if(i == str.size())                                 //if : not found return error
        return false;
    LOG_DEBUG(lstr.substr(0, i));
    StrIntFloat c(lstr.substr(0, i));                    //make a string for command
    cmd_map.insert( std::make_pair( "command", c) );    //insert command into map


    lstr = lstr.substr(i+2);                            //set lstring beyond command

    //loop until break or return
    for(;;)
    {
        int pos;
        cstr = lstr.c_str();
        i = strcspn(cstr, "=");                          //search of "="
        if(i == lstr.size())                            //if not found no more
            break;                                      //key=value pairs so break loop

        if(cstr[i+1] == '\"')                           //if value looks like a string
        {
            std::string tstr = lstr.substr(i+1);
            pos = get_end_of_quoted_string(tstr);       //find end of "
            if(pos == 0)                                //if value not string
                return false;                           //return error

            tstr = lstr.substr(0, i);                    //tstr is key
            LOG_DEBUG(tstr);


            std::string quote = lstr.substr(i+2, pos-2); //strip begin and end quotes
            quote = strUnEsq(quote);                    //clean any escape backslashes
            LOG_DEBUG(quote);
            StrIntFloat q(quote);                       //make string for value
            cmd_map.insert( make_pair(tstr, q));         //insert key,value into map
        }
        else{
            pos = strcspn(cstr+i+1, " ");                //find end of key=value pair

            LOG_DEBUG(lstr.substr(0, i));
            LOG_DEBUG(lstr.substr(i+1, pos));

            std::stringstream stream(lstr.substr(i+1, pos)); //string of value
            if(strcspn(stream.str().c_str(), ".")            //if value does not
               == stream.str().size())                      //contain . it is an int
            {
                int temp_i;
                stream >> temp_i;                           //convert str to int
                StrIntFloat temp(temp_i);                   //make int
                cmd_map.insert( make_pair(lstr.substr(0, i), temp)); //insert key,value
            }
            else{                                           //value contains .
                                                            //thus it is a float
                float temp_f;
                stream >> temp_f;                           //convert str to float
                StrIntFloat temp(temp_f);                   //make float
                cmd_map.insert( make_pair(lstr.substr(0, i), temp)); //insert key,value
            }
        }
        if(lstr.size() > i+2+pos)                           //more characters to process
            lstr = lstr.substr(i+2+pos);
        else
            break;                                          //no more to process
    }

    return true;
}


