#include <iostream>
#include <time.h>
#include <sstream>
#include <stdlib.h>
#include "config.h"
#include "logWriter.h"
#include "lassert.h"



void assert_throw(__const char *__assertion, __const char *__file,
                           unsigned int __line, __const char *__function)
{
    cerr_log_( __assertion, ASSERT_FAIL, __file, __function, __line);

}



#if 0 
#ifdef CONFIG_DEBUG
const LogLevel LOG_LEVEL = DEBUG;
#else
const LogLevel LOG_LEVEL = INFO;
#endif
#endif


bool logLevelIsValid(LogLevel level)
{
    switch (level)
    {
        case DEBUG:
        case INFO:
        case WARNING:
        case ERROR:
        case CRITICAL:
        case ASSERT_FAIL:
            return true;
            break;
        default:
            return false;
            break;
    }
}


const std::string logLevelStr(LogLevel level)
{
    switch (level)
    {
        case DEBUG:
            return "DEBUG: ";
        case INFO:
            return "INFO: ";
        case WARNING:
            return "WARNING: ";
        case ERROR:
            return "ERROR: ";
        case CRITICAL:
            return "CRITICAL: ";
        case ASSERT_FAIL:
            return "ASSERT_FAIL: ";
        default:
            return "INVALID LOG LEVEL: ";
    }
}



bool logLevelMatch(LogLevel level)
{
    if (level >= LOG_LEVEL && logLevelIsValid(level))
        return true;
    else
        return false;
}


const std::string log_(const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, const int lineNum)
{
    std::ostringstream logMsg;
    if (logLevelMatch(level))
    {
        time_t rawtime;
        struct tm * timeinfo;

        time( &rawtime );
        timeinfo = localtime(&rawtime);
        logMsg << logLevelStr(level) << msg << " --" << functionName <<  "() in " << fileName << ":" << " line " << lineNum << "-- " <<asctime(timeinfo); 

        // FIXME: send message to Core
    }

    return logMsg.str();
}

void cerr_log_( const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, const int lineNum)
{
    std::string err =  log_(msg,level,fileName,functionName,lineNum);

    std::cerr << err;
    if(level >= CRITICAL)
        throw(except(level,err));

}


