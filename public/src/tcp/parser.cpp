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

    for(unsigned int pos=0;pos < str.size();++pos)
    {
        char c = str[pos];
        if(c == '\\')
            out.append("\\\\");
        else if(c == '\"')
                out.append("\\\"");
        else
            out.append(1,c);          
    }
    return out;
}

std::string strUnEsq(const std::string& str)
{
    std::string out;

    for(unsigned int pos=0;pos < str.size();++pos)
    {
        char c = str[pos];
        char c2 = str[pos+1];
        if(c == '\\')
        {
            if(c2 == '\\')
            {
                out.append("\\");
                ++pos;
            }
        }
        else
            out.append(1,c);          
    }
    return out;
}

int get_end_of_quoted_string(const std::string& str)
{
    if(str[0] != '\"'){
        LOG_WARNING("String must start with \".");
        return 0;   
    }
    for(unsigned int pos=1; pos < str.size(); ++pos)
    {
        if(str[pos] == '\"')
            if(str[pos-1] != '\\')
                return pos+1;
    }

    LOG_WARNING("String has no terminating \".");
    return 0; 
}

bool stringify(std::map<std::string,StrIntFloat>& cmd_map, std::string& str)
{  
    typedef std::map<std::string,StrIntFloat>::iterator iter;
    str.clear();

    {
        iter it = cmd_map.find(std::string("command"));
        if(it != cmd_map.end())
        {
            std::string temp;
            if(!it->second.get(temp))
                return false;
            str.append(temp);
        }
        else
        {
            return false;
        }
    }
    str.append(":");

    for(iter it = cmd_map.begin();
            it != cmd_map.end(); ++it)
    {
        if(it->first.compare("command"))
        {
            str.append(" ");
            str.append(it->first);
            str.append("=");
            switch(it->second.type())
            {
                case 's':
                    {
                        std::string temp;
                        if(!it->second.get(temp))
                            return false;
                        str.append("\"");
                        temp = strEsq(temp);
                        str.append(temp);
                        str.append("\"");
                        break;
                    }
                case 'i':
                    {
                        int temp;
                        if(!it->second.get(temp))
                            return false;
                        std::stringstream sstream;
                        sstream << temp;
                        str.append(sstream.str());
                        break;
                    }
                case 'f':
                    {
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
    
    str.append("\r\\\n");

    return true;
}


bool tokenize(const std::string& str, std::map<std::string,StrIntFloat> &cmd_map) 
{
    unsigned int i;
    const char *cstr = str.c_str();
    std::string lstr = str;
    cmd_map.clear();
    i = strcspn(cstr,":");
    if(i == str.size())
        return false;
    LOG_DEBUG(lstr.substr(0,i));  
    StrIntFloat c(lstr.substr(0,i));
    cmd_map.insert( std::make_pair( "command", c) );
    

    lstr = lstr.substr(i+2);
    for(;;)
    {
        int pos;
        cstr = lstr.c_str();
        i = strcspn(cstr,"=");
        if(i == lstr.size())
            break;

        if(cstr[i+1] == '\"')
        {
            std::string tstr = lstr.substr(i+1);
            pos = get_end_of_quoted_string(tstr);
            tstr = lstr.substr(0,i);
            LOG_DEBUG(tstr);

            if(pos == 0)
                return false;

            std::string quote = lstr.substr(i+2,pos-2); //strip quotes
            quote = strUnEsq(quote);
            LOG_DEBUG(quote);
            StrIntFloat q(quote);
            cmd_map.insert( make_pair(tstr,q));
        }
        else
        {
            pos = strcspn(cstr+i+1," ");
            
            LOG_DEBUG(lstr.substr(0,i));
            LOG_DEBUG(lstr.substr(i+1,pos));
            std::stringstream stream(lstr.substr(i+1,pos));
            if(strcspn(stream.str().c_str(),".") == stream.str().size())
            {
                int temp_i;
                stream >> temp_i;
                StrIntFloat temp(temp_i);
                cmd_map.insert( make_pair(lstr.substr(0,i),temp));
            }
            else
            {
                float temp_f;
                stream >> temp_f;
                StrIntFloat temp(temp_f);
                cmd_map.insert( make_pair(lstr.substr(0,i),temp));

            }
        }
        if(lstr.size() > i+2+pos)
            lstr = lstr.substr(i+2+pos); 
        else
            break;

    }
    return true;
}



