#include "parser.h"
#include "logWriter.h"
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

int get_end_of_quoted_string(std::string& str)
{
    if(str[0] != '\"'){
        LOG_WARNING("String must start with \".");
        return 0;   
    }
    for(unsigned int pos=1; pos < str.size(); ++pos)
    {
        if(str[pos] == '\"')
            if(str[pos-1] != '\\')
                return pos;
    }

    LOG_WARNING("String has no terminating \".");
    return 0; 
}


