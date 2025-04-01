#ifndef FT_PING_H
#define FT_PING_H
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <error.h>
#include <math.h>

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

#define PACKET_SIZE IP_HEADER_SIZE + ICMP_PACKET_SIZE
#define IP_HEADER_SIZE sizeof(struct ip)
#define ICMP_PACKET_SIZE sizeof(struct icmphdr) + ICMP_PAYLOAD_SIZE
#define ICMP_PAYLOAD_SIZE ICMP_PAYLOAD_CHUNK_SIZE + sizeof(struct timeval)

#define ICMP_PAYLOAD_CHUNK "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f" \
                           "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f" \
                           "\x20\x21\x22\x23\x24\x25\x26\x27"
#define ICMP_PAYLOAD_CHUNK_SIZE 40

struct s_ping
{
    char *hostarg;
    char hostip[INET_ADDRSTRLEN];
    struct addrinfo *host;
    bool verbose;
    int ttl;

    int sockfd;
    pid_t self_pid;
    bool loop;

	int sent_packets;
	int received_packets;
    float round_trip_min;
    float round_trip_max;
    float round_trip_sigma;
    float round_trip_squared_sigma;
};

extern struct s_ping g_ping;

void signal_handler(int);
void print_stats();

void print_header_output();
uint16_t calculate_checksum(uint16_t *packet, int len);
float calculate_round_trip(unsigned char *buffer);

// utils
void print_struct_data();
void hex_print_data(unsigned char *packet, int len);
void hex_print_icmp_packet_data(unsigned char *packet, int len);

void ft_ping();
