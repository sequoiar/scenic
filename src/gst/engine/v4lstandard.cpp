

// largely taken from v4l2-ctl.cpp

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <map>
#include <cassert>

static void print_std(const char *prefix, const char *stds[], unsigned long long std)
{
    int first = 1;

    printf("\t%s-", prefix);
    while (*stds) 
    {
        if (std & 1) 
        {
            if (!first)
                printf("/");
            first = 0;
            printf("%s", *stds);
        }
        stds++;
        std >>= 1;
    }
    printf("\n");
}

static int doioctl(int fd, int request, void *parm, const char *name)
{
    int retVal;

    retVal = ioctl(fd, request, parm);
    printf("%s: ", name);
    if (retVal < 0)
        printf("failed: %s\n", strerror(errno));
    else
        printf("ok\n");

    return retVal;
}


bool checkFormat(const char * expected)
{

    v4l2_std_id std;
    int fd = -1;

    std::map<const char *, unsigned long long> FORMATS;
    FORMATS["PAL"] = 0xfff;
    FORMATS["NTSC"] = 0xf000;
    FORMATS["SECAM"] = 0xff0000;
    FORMATS["ATSC/HDTV"] =  0xf000000;
    
    char *device = strdup("/dev/video0");
    if ((fd = open(device, O_RDWR)) < 0) 
    {
        fprintf(stderr, "Failed to open %s: %s\n", device,
                strerror(errno));
        exit(1);
    }
    free(device);


    if (doioctl(fd, VIDIOC_G_STD, &std, "VIDIOC_G_STD") == 0) 
    {
        static const char *pal[] = {
            "B", "B1", "G", "H", "I", "D", "D1", "K",
            "M", "N", "Nc", "60",
            NULL
        };
        static const char *ntsc[] = {
            "M", "M-JP", "443", "M-KR",
            NULL
        };
        static const char *secam[] = {
            "B", "D", "G", "H", "K", "K1", "L", "Lc",
            NULL
        };
        static const char *atsc[] = {
            "ATSC-8-VSB", "ATSC-16-VSB",
            NULL
        };

        printf("Video Standard = 0x%08llx\n", (unsigned long long)std);

        if (std & FORMATS["PAL"]) {
            print_std("PAL", pal, std);
        }
        if (std & FORMATS["NTSC"]) {
            print_std("NTSC", ntsc, std >> 12);
        }
        if (std & FORMATS["SECAM"]) {
            print_std("SECAM", secam, std >> 16);
        }
        if (std & FORMATS["ATSC/HDTV"]) {
            print_std("ATSC/HDTV", atsc, std >> 24);
        }

    }
}


int main(int argc, const char* argv[])
{
    if (argc != 2)
    { 
        std::cout << "Usage: v4l2standard <FORMAT>" << std::endl;
        return 1;
    }

    if (checkFormat(argv[1]))
        std::cout << "Correct format " << argv[1] << std::endl;
    else 
        std::cout << "Incorrect format " << argv[1] << std::endl;

    return 0;
}

