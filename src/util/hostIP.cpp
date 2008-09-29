
#include "hostIP.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

const char *get_host_ip()
{
    int i;
    char *ip = 0;
    int s = socket (PF_INET, SOCK_STREAM, 0);
    if(s == -1)
        return ip;

    for (i = 1;; i++)
    {
        struct ifreq ifr;
        struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;

        ifr.ifr_ifindex = i;
        if (ioctl (s, SIOCGIFNAME, &ifr) < 0)
            break;
        /* now ifr.ifr_name is set */
        if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
            continue;
        ip = inet_ntoa (sin->sin_addr);         // seems to be thread safe
                                                // but not reentrant
                                                // under libc6 2+
    }

    close (s);
    return ip;
}
