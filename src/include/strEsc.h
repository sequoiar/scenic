#include <string>


static std::string strEsq(std::string& str)
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
