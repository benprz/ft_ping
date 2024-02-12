// inetutils-2.0 (ping -V)
// -v options
// simple ipv4 (address/hostname) as parameters
// manage FQDN without doing DNS resolution in the packet return

// printf family
/*
◦gettimeofday.
◦exit.
◦inet_ntop. / inet_pton.
◦ntohs / htons.
◦signal / alarm / usleep.
◦socket / setsockopt / close.
◦sendto / recvmsg.
◦getpid / getuid.
◦getaddrinfo / getnameinfo / freeaddrinfo.
◦strerror / gai_strerror.
◦printf and its family.
◦Your libft functions.
*/

#include "libft.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//getaddrinfo
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//inet_pton
#include <arpa/inet.h>

struct s_ping
{
    int verbose;
    char *host;
};

struct s_ping g_ping = {0, NULL};

int parse_host(char *host)
{
    // check if host is a valid ipv4 address
    // if (inet_pton(AF_INET, host, NULL) == 0)
    //     return (1);

    // g_ping.host = host;
    
    return 0;
}

int parse_args(char **argv)
{
    for (int i = 1; argv[i]; i++)
    {
        if (ft_strcmp(argv[i], "-v") == 0)
        {
            printf("Verbose mode\n");
            g_ping.verbose = 1;
        }
        else if (parse_host(argv[i]) == 1)
        {
            printf("Invalid host\n");
            return (1);
        }
    }
    return (0);
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3)
    {
        printf("Usage: ping [-v] destination\n");
        return (1);
    }

    if (getuid() != 0)
    {
        printf("You must be root to use ping\n");
        return (1);
    }

    if (parse_args(argv) == 1)
    {
        return (1);
    }

    // printf("Host: %s\n", g_ping.host);
    printf("Verbose: %d\n", g_ping.verbose);

    return (0);
}