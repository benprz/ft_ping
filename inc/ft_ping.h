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

#define PACKET_SIZE IP_HEADER_SIZE + ICMP_PACKET_SIZE
#define IP_HEADER_SIZE sizeof(struct ip)
#define ICMP_PACKET_SIZE 64
#define ICMP_PAYLOAD_SIZE ICMP_PACKET_SIZE - sizeof(struct icmphdr)
#define ICMP_PAYLOAD_CHUNK_SIZE ICMP_PAYLOAD_SIZE - sizeof(struct timeval)

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