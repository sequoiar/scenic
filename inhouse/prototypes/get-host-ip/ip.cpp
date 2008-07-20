///http://www.hungry.com/~alves/local-ip-in-C.html
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
  int i;
  int s = socket (PF_INET, SOCK_STREAM, 0);

  for (i=1;;i++)
	{
	  struct ifreq ifr;
	  struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;
	  char *ip;

	  ifr.ifr_ifindex = i;
	  if (ioctl (s, SIOCGIFNAME, &ifr) < 0)
		break;

	  /* now ifr.ifr_name is set */
	  if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
		continue;

	  ip = inet_ntoa (sin->sin_addr);
	  printf ("%s\n", ip);
	}

  close (s);
  return 0;
}




#if 0

 

The other day, I was home sick, and I thought to myself "Wouldn*t it be fun to figure out how to find a machine*s local IP addresses from C?" It turns out that it isn*t fun. I got the usual set of goony responses like "Read it from /proc/something!" or "Just fork ifconfig and parse the output!". I*ve collected some of my discoveries here: The first example I found did something like this:

    #include <sys/ioctl.h>
    #include <net/if.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <netdb.h>
    #include <errno.h>
    #include <string.h>

    int main (int argc, char *argv[])
    {
      struct hostent *he;
      struct sockaddr_in sin;
      char machine_name[ 300 ];

      if (gethostname (machine_name, sizeof (machine_name)))
        {
          perror ("gethostname");
          return -1;
        }

     again:

      he = gethostbyname (machine_name);
      if (he == NULL)
        {
          if (errno == TRY_AGAIN)
          goto again;

          perror ("gethostbyname");
          return -1;
        }

      bzero (&sin, sizeof (struct sockaddr_in));
      sin.sin_family = AF_INET;
      sin.sin_port = htons (80);
      bcopy (he->h_addr_list[ 0 ], &sin.sin_addr, he->h_length);

      printf ("%s\n", inet_ntoa (sin.sin_addr));

      return 0;
    }

This is pretty easy, but it talks with bind and relies on DNS being correct. This could be made more thorough, but I wont bother. The next working example I found did this:

    #include <sys/ioctl.h>
    #include <net/if.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <stdio.h>

    int main (int argc, char *argv[])
    {
      int i;
      int s = socket (PF_INET, SOCK_STREAM, 0);

      for (i=1;;i++)
        {
          struct ifreq ifr;
          struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;
          char *ip;

          ifr.ifr_ifindex = i;
          if (ioctl (s, SIOCGIFNAME, &ifr) < 0)
            break;

          /* now ifr.ifr_name is set */
          if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
            continue;

          ip = inet_ntoa (sin->sin_addr);
          printf ("%s\n", ip);
        }

      close (s);
      return 0;
    }

This sets the index field in the ifreq structure and calls an ioctl to fill in the name of the interface corresponding to that index. It then does a second ioctl call to get the ip address associated with the interface of that name. This is neat, except it only works under linux. Faried showed me some code that used getifaddrs, which I boiled down to this. Later, Ryan Davis pointed out a couple bugs in what I originally put here. Even later, Andy Green pointed out that I was calling freeifaddrs on the end of the list instead of the beginning. Someone called "Heart Breaker" noticed I was dereferencing ifa->ifa_addr before making sure it wasn*t NULL.

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <ifaddrs.h>
    #include <string.h>

    int main (int argc, char *argv[])
    {
      struct ifaddrs *ifa = NULL, *ifp = NULL;

      if (getifaddrs (&ifp) < 0)
        {
          perror ("getifaddrs");
          return 1;
        }

      for (ifa = ifp; ifa; ifa = ifa->ifa_next)
        {
          char ip[ 200 ];
          socklen_t salen;

          if(! ifa->ifa_addr)
            continue;
          /* if (strcmp(ifa->ifa_name,"ppp0") == 0) continue; */
          if (ifa->ifa_addr->sa_family == AF_INET)
            salen = sizeof (struct sockaddr_in);
          else if (ifa->ifa_addr->sa_family == AF_INET6)
            salen = sizeof (struct sockaddr_in6);
          else
            continue;

          if (getnameinfo (ifa->ifa_addr, salen,
                           ip, sizeof (ip), NULL, 0, NI_NUMERICHOST) < 0)
            {
              perror ("getnameinfo");
              continue;
            }
          printf ("%s\n", ip);

        }

      freeifaddrs (ifp);

      return 0;
    }
        

This one seems to be the most portable (meaning it works for Linux, FreeBSD and OSX -- and vmware, says HB). It works for IPv4 and IPv6. A couple things bother me about it.

    * I feel like there should be some way to set salen without iterating over all the address families that I*m interested in. Is there?
    * What*s it talking to? Why? As getifaddrs is called, strace dumps this:

          socket(PF_NETLINK, SOCK_RAW, 0)         = 4
          bind(4, {sa_family=AF_NETLINK, pid=0, groups=00000000}, 12) = 0
          getsockname(4, {sa_family=AF_NETLINK, pid=8565, groups=00000000}, [12]) = 0
          time(NULL)                              = 1146194650
          sendto(4, "\24\0\0\0\22\0\1\3\332\212QD\0\0\0\0\0\0\0\0", 20, 0, {sa_family=AF_NETLINK, pid=0, groups=00000000}, 12) = 20
          recvmsg(4, {msg_name(12)={sa_family=AF_NETLINK, pid=0, groups=00000000}, msg_iov(1)=[{"\344\0\0\0\20\0\2\0\332\212QDu!\0\0\0003\4\3\1\0\0\0I\0"..., 4096}], msg_controllen=0, msg_flags=0}, 0) = 924
          brk(0)                                  = 0x804a000
          brk(0x806b000)                          = 0x806b000
          recvmsg(4, {msg_name(12)={sa_family=AF_NETLINK, pid=0, groups=00000000}, msg_iov(1)=[{"\24\0\0\0\3\0\2\0\332\212QDu!\0\0\0\0\0\0\1\0\0\0I\0\0"..., 4096}], msg_controllen=0, msg_flags=0}, 0) = 20
          sendto(4, "\24\0\0\0\26\0\1\3\333\212QD\0\0\0\0\0\0\0\0", 20, 0, {sa_family=AF_NETLINK, pid=0, groups=00000000}, 12) = 20
          recvmsg(4, {msg_name(12)={sa_family=AF_NETLINK, pid=0, groups=00000000}, msg_iov(1)=[{"D\0\0\0\24\0\2\0\333\212QDu!\0\0\2\30\200\0\3\0\0\0\10"..., 4096}], msg_controllen=0, msg_flags=0}, 0) = 68
          recvmsg(4, {msg_name(12)={sa_family=AF_NETLINK, pid=0, groups=00000000}, msg_iov(1)=[{"@\0\0\0\24\0\2\0\333\212QDu!\0\0\n\200\200\376\1\0\0\0"..., 4096}], msg_controllen=0, msg_flags=0}, 0) = 320
          recvmsg(4, {msg_name(12)={sa_family=AF_NETLINK, pid=0, groups=00000000}, msg_iov(1)=[{"\24\0\0\0\3\0\2\0\333\212QDu!\0\0\0\0\0\0\1\0\0\0\24\0"..., 4096}], msg_controllen=0, msg_flags=0}, 0) = 20
          close(4)                                = 0

See also: getting a MAC address from an interface 
#endif
