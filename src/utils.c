#include "ft_ping.h"

void print_header_output() {
	printf("PING %s (%s): %ld data bytes", g_ping.hostarg, g_ping.hostip, ICMP_PAYLOAD_SIZE);
	if (g_ping.verbose)
		printf(", id 0x%04x = %u", g_ping.self_pid, g_ping.self_pid);
	printf("\n");
}

// The checksum field is the 16 bit one's complement of the one's complement sum of all 16 bit words in the header.
// The ones' complement of a binary number is the value obtained by inverting (flipping) all the bits in the binary representation of the number.
uint16_t calculate_checksum(uint16_t *packet, int len)
{
	uint32_t sum = 0;

	// addition of 16-bits words
	while (len > 1)
	{
		sum += *packet++;
		len -= 2;
	}
	// if length is odd, add the last byte
	if (len == 1)
		sum += *(uint8_t *)packet;

	// add carry (high 16 bits to low 16 bits) until carry is zero
	while (sum >> 16)
		sum = (sum >> 16) + (sum & 0xffff);

	// return the one's complement of sum
	return (uint16_t)(~sum);
}

float calculate_round_trip(unsigned char *buffer)
{
	struct timeval *tv_packet = (struct timeval*)(buffer + sizeof(struct ip) + sizeof(struct icmphdr));
	struct timeval tv_now;

	gettimeofday(&tv_now, NULL);
	return (tv_now.tv_sec - tv_packet->tv_sec) * 1000.0 + (tv_now.tv_usec - tv_packet->tv_usec) / 1000.0;
}

// void print_struct_data()
// {
// 	printf("\n");
// 	// print g_ping data structure
// 	for (struct addrinfo *p = g_ping.host; p != NULL; p = p->ai_next)
// 	{
// 		void *addr;
// 		char *ipver;

// 		// get the pointer to the address itself,
// 		// different fields in IPv4 and IPv6:
// 		if (p->ai_family == AF_INET)
// 		{ // IPv4
// 			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
// 			addr = &(ipv4->sin_addr);
// 			ipver = "IPv4";
// 		}
// 		else
// 		{ // IPv6
// 			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
// 			addr = &(ipv6->sin6_addr);
// 			ipver = "IPv6";
// 		}

// 		char ipstr[INET6_ADDRSTRLEN];
// 		// convert the IP to a string and print it:
// 		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
// 		printf("  %s: %s\n", ipver, ipstr);
// 	}
// 	printf("  verbose: %d\n", g_ping.verbose);
// 	printf("\n");
// }

// void hex_print_data(unsigned char *packet, int len)
// {
// 	// print the packet data with different color for each header
// 	printf("\033[0;34m");
// 	for (int i = 0; i < 8; i++)
// 	{
// 		printf("%02x ", packet[i]);
// 	}
// 	printf("\033[0;35m");
// 	for (int i = 8; i < 24; i++)
// 	{
// 		printf("%02x ", packet[i]);
// 	}
// 	printf("\033[0;36m");
// 	for (int i = 24; i < len; i++)
// 	{
// 		printf("%02x ", packet[i]);
// 	}
// 	printf("\033[0m\n");
// }

// void hex_print_icmp_packet_data(unsigned char *packet, int len)
// {
// 	// separate the headers
// 	struct ip *ip_hdr = (struct ip *)packet;

// 	// print the packet data with different color for each header
// 	printf("\033[0;31m");
// 	for (int i = 0; i < IP_HEADER_SIZE; i++)
// 	{
// 		printf("%02x ", packet[i]);
// 	}
// 	printf("\033[0m");
// 	printf("\033[0;34m");
// 	for (int i = IP_HEADER_SIZE; i < len; i++)
// 	{
// 		printf("%02x ", packet[i]);
// 	}
// 	printf("\033[0m\n");

// }
