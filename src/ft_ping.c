#include "ft_ping.h"

#include <sys/select.h>

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
	for (int i = 0; i < (ip_hdr->ip_hl << 2); i++)
	{
		printf("%02x ", packet[i]);
	}
	printf("\033[0m");
	printf("\033[0;34m");
	for (int i = (ip_hdr->ip_hl << 2); i < len; i++)
	{
		printf("%02x ", packet[i]);
	}
	printf("\033[0m\n");

}

float calculate_delay(unsigned char *buffer)
{
	struct timeval *tv_packet = (struct timeval*)(buffer + sizeof(struct ip) + sizeof(struct icmphdr));
	struct timeval tv_now;

	gettimeofday(&tv_now, NULL);
	return tv_now.tv_sec - tv_packet->tv_sec + (tv_now.tv_usec - tv_packet->tv_usec) / 1000.0;
}

void get_reply(int sockfd, int seq)
{
	// printf("Getting reply....");
	unsigned char buffer[1024];
	struct sockaddr_storage src_addr;
	socklen_t src_addr_len = sizeof(src_addr);
	ssize_t count;

	if ((count = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &src_addr_len)) < 0)
	{
		error(EXIT_FAILURE, errno, "recvfrom");
	}

	struct ip *ip_hdr = (struct ip *)buffer;
	struct icmphdr *icmp_hdr = (struct icmphdr *)(buffer + (ip_hdr->ip_hl << 2));

	// hex_print_icmp_packet_data(buffer, count);
	// print_ethernet_header(buffer, count);
	//printf("ICMP_REPLY\n");
	// printf("icmp_hdr->type=%d\n", icmp_hdr->type);
	// printf("icmp_hdr->un.echo.id=%d\n", ntohs(icmp_hdr->un.echo.id));
	// printf("icmp_hdr->un.echo.sequence=%d | seq=%d\n", ntohs(icmp_hdr->un.echo.sequence), seq);
	// printf("icmp_hdr->checksum=0x%x\n", ntohs(icmp_hdr->checksum));

	if (icmp_hdr->type == ICMP_ECHOREPLY &&
		ntohs(icmp_hdr->un.echo.id) == g_ping.self_pid &&
		ntohs(icmp_hdr->un.echo.sequence) == seq)
	{
		printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
			   count - sizeof(struct ip), g_ping.hostip, seq, ip_hdr->ip_ttl, calculate_delay(buffer));
	}
}

void send_request(int sockfd, unsigned char *packet)
{
	if (sendto(sockfd, packet, ICMP_PACKET_SIZE, 0, g_ping.host->ai_addr, g_ping.host->ai_addrlen) < 0)
	{
		error(EXIT_FAILURE, errno, "sendto");
	}
}

int send_echo_requests(int sockfd)
{
	unsigned char packet[ICMP_PACKET_SIZE];

	unsigned char chunk[ICMP_PAYLOAD_CHUNK_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
									  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
									  20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
									  30, 31, 32, 33, 34, 35, 36, 37, 38, 39};

	struct icmphdr icmp_hdr = {
		.type = ICMP_ECHO, // no need htons coz its a byte
		.code = htons(0), // same
		.un.echo.id = htons(g_ping.self_pid),
	};

	printf("PING %s (%s): %ld data bytes\n", g_ping.hostarg, g_ping.hostip, ICMP_PAYLOAD_SIZE);

	fd_set readfds;
	int select_ret;
	FD_ZERO(&readfds);

	int seq = 0;
	while (1)
	{
		FD_SET(sockfd, &readfds);
		bzero(packet, sizeof(packet));
		icmp_hdr.un.echo.sequence = htons(seq);
		memcpy(packet, &icmp_hdr, sizeof(struct icmphdr));
		gettimeofday((struct timeval *)(packet + sizeof(struct icmphdr)), NULL);
		memcpy(packet + sizeof(struct icmphdr) + sizeof(struct timeval), chunk, ICMP_PAYLOAD_CHUNK_SIZE);

		uint16_t checksum = calculate_checksum((uint16_t *)packet, ICMP_PACKET_SIZE);
		//big endian
		packet[2] = checksum & 0xff;
		packet[3] = checksum >> 8;

		// printf("\n");
		// printf("ICMP_ECHO\n");
		// printf("icmp_hdr->type=%d\n", icmp_hdr.type);
		// printf("icmp_hdr->un.echo.id=%d\n", ntohs(icmp_hdr.un.echo.id));
		// printf("icmp_hdr->un.echo.sequence=%d \n", ntohs(icmp_hdr.un.echo.sequence));
		// printf("icmp_hdr->checksum=0x%x\n", ntohs(icmp_hdr.checksum));

		// hex_print_data(packet, sizeof(struct icmphdr) + ICMP_PAYLOAD_SIZE);
		send_request(sockfd, packet);
		if ((select_ret = select(sockfd + 1, &readfds, NULL, NULL, NULL)) < 0)
			error(EXIT_FAILURE, errno, "select");
		else if (select_ret == 0)
			printf("Request timeout for icmp_seq %d\n", seq);
		else
			get_reply(sockfd, seq);
		sleep(1);

		seq++;
	}
}

int ft_ping()
{
	int sockfd;
	// print_struct_data();

	if ((sockfd = socket(g_ping.host->ai_family, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		error(EXIT_FAILURE, errno, "socket");
	}

	g_ping.self_pid = getpid();

	return send_echo_requests(sockfd);
}