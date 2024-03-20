#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <error.h>

//getaddrinfo
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//inet_pton
#include <arpa/inet.h>

//icmp_hdr
#include <netinet/ip_icmp.h>

//gettimeofday
#include <sys/time.h>

#define UNUSED(x) (void)x

#define PAYLOAD_SIZE 56
#define PAYLOAD_CHUNK_SIZE PAYLOAD_SIZE - sizeof(struct timeval)

struct s_ping
{
    char *hostarg;
    char hostip[INET_ADDRSTRLEN];
    struct addrinfo *host;
    bool verbose;
    pid_t self_pid;
};

extern struct s_ping g_ping;

int ft_ping();