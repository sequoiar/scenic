#include <glib.h>
#include <stdio.h>
#include <map>
#include <string>
#include "tcp/parser.h"
#include "logWriter.h"
#include "gutil/strIntFloat.h"

std::string str = "init: it=103945 mystring=\"\\\"bar you\\\"\" Hellotab=1.1";


int main(int, char**)
{
    std::map<std::string, StrIntFloat> mymap;
    if(tokenize(str, mymap))
        LOG_DEBUG("Success")
        else
            LOG_DEBUG("Fail")

            std::string out;
    if(stringify(mymap, out))
        LOG_DEBUG(out)
        else
            LOG_DEBUG("Fail stringify")


            return 0;
}


