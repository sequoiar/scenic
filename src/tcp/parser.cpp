#include "parser.h"


std::string strEsq(std::string& str)
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

std::string strUnEsq(std::string& str)
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

