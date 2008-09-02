#include <glib.h>
#include <stdio.h>
#include <map>
#include <string>
#include "tcp/parser.h"
#include "logWriter.h"

std::string str = "init: mystring=\"\\\"bar you\\\"\" Hellotab=1";


int main(int, char**)
{
    std::map<std::string,std::string> mymap;
    if(tokenize(str,mymap))
    {
        LOG_DEBUG("Success");
    }
    else
    {
        LOG_DEBUG("Fail");
    }

    return 0;
}


