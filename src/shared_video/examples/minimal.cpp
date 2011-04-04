/**
 * Minimal example of how to build and link to Scenic's shared_video library.
 * 
 * g++ -Wall -Werror `pkg-config --cflags --libs scenic` -o minimal minimal.cpp
 */

#include <sharedVideoBuffer.h>

int main()
{
    SharedVideoBuffer buffer(320, 240);
    return 0;
}

