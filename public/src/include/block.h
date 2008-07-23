#ifndef _BLOCK_H_
#define _BLOCK_H_

#define BLOCKING 1

#if BLOCKING
#define BLOCK() std::cout.flush();                              \
    std::cout << __FILE__ << ":" << __LINE__        \
              << ": blocking, enter any key." << std::endl;   \
    std::cin.get()
#else
#define BLOCK()
#endif

#endif  // _BLOCK_H_
