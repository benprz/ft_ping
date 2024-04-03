#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
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

#define ICMP_PAYLOAD_CHUNK "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f" \
                           "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f" \
                           "\x20\x21\x22\x23\x24\x25\x26\x27"
#define ICMP_PAYLOAD_CHUNK_SIZE ICMP_PAYLOAD_SIZE - sizeof(struct timeval)

struct s_ping
{
    char *hostarg;
    char hostip[INET_ADDRSTRLEN];
    struct addrinfo *host;
    bool verbose;

    int sockfd;
    pid_t self_pid;
    bool loop;

	int sent_packets;
	int received_packets;
};

extern struct s_ping g_ping;


void signal_handler(int);
void print_stats();

void ft_ping();