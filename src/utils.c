#include "ft_ping.h"

void print_struct_data()
{
	printf("\n");
	// print g_ping data structure
	for (struct addrinfo *p = g_ping.host; p != NULL; p = p->ai_next)
	{
		void *addr;
		char *ipver;

		// get the pointer to the address itself,
		// different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET)
		{ // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		}
		else
		{ // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}

		char ipstr[INET6_ADDRSTRLEN];
		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf("  %s: %s\n", ipver, ipstr);
	}
	printf("  verbose: %d\n", g_ping.verbose);
	printf("\n");
}

void hex_print_data(unsigned char *packet, int len)
{
	// print the packet data with different color for each header
	printf("\033[0;34m");
	for (int i = 0; i < 8; i++)
	{
		printf("%02x ", packet[i]);
	}
	printf("\033[0;35m");
	for (int i = 8; i < 24; i++)
	{
		printf("%02x ", packet[i]);
	}
	printf("\033[0;36m");
	for (int i = 24; i < len; i++)
	{
		printf("%02x ", packet[i]);
	}
	printf("\033[0m\n");
}

void hex_print_icmp_packet_data(unsigned char *packet, int len)
{
	// separate the headers
	struct ip *ip_hdr = (struct ip *)packet;

	// print the packet data with different color for each header
	printf("\033[0;31m");
	for (int i = 0; i < IP_HEADER_SIZE; i++)
	{
		printf("%02x ", packet[i]);
	}
	printf("\033[0m");
	printf("\033[0;34m");
	for (int i = IP_HEADER_SIZE; i < len; i++)
	{
		printf("%02x ", packet[i]);
	}
	printf("\033[0m\n");

}
