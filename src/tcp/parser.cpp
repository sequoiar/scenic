#include "parser.h"
#include "logWriter.h"



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

bool tokenize(const std::string& str, std::map<std::string,std::string> &cmd_map) 
{
    unsigned int i;
    const char *cstr = str.c_str();
    std::string lstr = str;
    cmd_map.clear();
    i = strcspn(cstr,":");
    if(i == str.size())
        return false;
    LOG_DEBUG(lstr.substr(0,i));  
    cmd_map.insert( make_pair( "command", lstr.substr(0,i)) );

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

            std::string quote = lstr.substr(i+2,pos-2);
            quote = strUnEsq(quote);
            LOG_DEBUG(quote);

            cmd_map.insert( make_pair(tstr,quote));
        }
        else
        {
            pos = strcspn(cstr+i+1," ");
            
            LOG_DEBUG(lstr.substr(0,i));
            LOG_DEBUG(lstr.substr(i+1,pos));

            cmd_map.insert( make_pair(lstr.substr(0,i),lstr.substr(i+1,pos)));
        }
        if(lstr.size() > i+2+pos)
            lstr = lstr.substr(i+2+pos); 
        else
            break;

    }
    return true;
}



