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

bool tokenize(std::string& str, std::map<std::string,std::string> &cmd_map) 
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

    lstr = lstr.substr(i+1);
    for(;;)
    {
        cstr = lstr.c_str();
        i = strcspn(cstr,"=");
        if(i == lstr.size())
            break;
    
        if(cstr[i+2] == '\"')
        {
            std::string tstr = lstr.substr(i+2);
            int pos = get_end_of_quoted_string(tstr);
            if(pos == 0)
                return false;

            std::string quote = lstr.substr(i+2,pos);
            LOG_DEBUG(quote);

            cmd_map.insert( make_pair(lstr.substr(0,i),quote));
        }
        else
        {
            int pos = strcspn(cstr+i+2," ");
            
            LOG_DEBUG(lstr.substr(0,i));


            LOG_DEBUG(lstr.substr(i+2,pos));


            cmd_map.insert( make_pair(lstr.substr(0,i),lstr.substr(i+2,pos)));




        }
        

    }
    return true;
}



