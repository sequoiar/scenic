#ifndef _LOG_WRITER_H_
#define _LOG_WRITER_H_

#include <iostream>

#define LOGGING 1

#if LOGGING
    #define LOG(x) \
                std::cerr << std::endl; \
                std::cerr << __FILE__ << ":" << __LINE__ << ": " << x; \
                std::cerr << " at " << __TIME__ << " on " << __DATE__ << std::endl
#else
    #define LOG(x)
#endif

#endif //  _LOG_WRITER_H_
