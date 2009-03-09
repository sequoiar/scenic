

// largely taken from v4l2-ctl.cpp

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>  // for O_RDWR
#include <cerrno>
#include <iostream>
#include <map>
#include <cassert>

#include "v4l2util.h"

static int doioctl(int fd, int request, void *parm, const char *name)
{
    int retVal;

    retVal = ioctl(fd, request, parm);
    std::cout << name << ": ";
    if (retVal < 0)
        std::cout << "failed: " << strerror(errno) << std::endl;
    //else
        // printf("ioctl ok\n");

    return retVal;
}


static bool v4l2util::checkStandard(const std::string &expected)
{

    bool result = false;
    v4l2_std_id std;
    int fd = -1;

    std::map<std::string, unsigned long long> FORMATS;
    FORMATS["PAL"] = 0xfff;
    FORMATS["NTSC"] = 0xf000;
    FORMATS["SECAM"] = 0xff0000;
    FORMATS["ATSC/HDTV"] =  0xf000000;
    
    char *device = strdup("/dev/video0");
    if ((fd = open(device, O_RDWR)) < 0) 
    {
        std::cerr << "Failed to open " << device << ": " << strerror(errno) << std::endl;
        exit(1);
    }
    free(device);

    if (doioctl(fd, VIDIOC_G_STD, &std, "VIDIOC_G_STD") == 0) 
    {
        for (std::map<std::string, unsigned long long>::const_iterator iter = FORMATS.begin();
                iter != FORMATS.end(); ++iter)
            if (std & (*iter).second)
                result = (result || (expected == (*iter).first));
    }
    return result;
}


#if 0
int main(int argc, const char* argv[])
{
    if (argc != 2)
    { 
        std::cout << "Usage: v4l2standard <FORMAT>" << std::endl;
        return 1;
    }

    std::string format(argv[1]);

    if (v4l2util::checkStandard(format))
        std::cout << "Correct format " << format << std::endl;
    else 
        std::cout << "Incorrect format " << format << std::endl;

    return 0;
}
#endif
