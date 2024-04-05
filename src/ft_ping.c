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

float calculate_round_trip(unsigned char *buffer)
{
	struct timeval *tv_packet = (struct timeval*)(buffer + sizeof(struct ip) + sizeof(struct icmphdr));
	struct timeval tv_now;

	gettimeofday(&tv_now, NULL);
	return tv_now.tv_sec - tv_packet->tv_sec + (tv_now.tv_usec - tv_packet->tv_usec) / 1000.0;
}

int get_reply(int seq)
{
	// printf("Getting reply....");
	unsigned char buffer[1024];
	struct sockaddr_storage src_addr;
	socklen_t src_addr_len = sizeof(src_addr);
	ssize_t count;

	if ((count = recvfrom(g_ping.sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &src_addr_len)) < 0)
	{
		error(EXIT_FAILURE, errno, "recvfrom");
	}
	struct ip *ip_hdr = (struct ip *)buffer;
	struct icmphdr *icmp_hdr = (struct icmphdr *)(buffer + (ip_hdr->ip_hl << 2));

	// hex_print_icmp_packet_data(buffer, count);
	// printf("ICMP_REPLY\n");
	// printf("icmp_hdr->type=%d\n", icmp_hdr->type);
	// printf("icmp_hdr->un.echo.id=%d\n", ntohs(icmp_hdr->un.echo.id));
	// printf("icmp_hdr->un.echo.sequence=%d | seq=%d\n", ntohs(icmp_hdr->un.echo.sequence), seq);
	// printf("icmp_hdr->checksum=0x%x\n", ntohs(icmp_hdr->checksum));

	// if (icmp_hdr->type == ICMP_ECHOREPLY)
	// 	printf("type yes\n");
	// if (ntohs(icmp_hdr->un.echo.id) == g_ping.self_pid)
	// 	printf("id yes\n");
	// if (ntohs(icmp_hdr->un.echo.sequence) == seq)
	// 	printf("seq yes\n");
	if (icmp_hdr->type == ICMP_ECHOREPLY &&
		ntohs(icmp_hdr->un.echo.id) == g_ping.self_pid)// &&
		// ntohs(icmp_hdr->un.echo.sequence) == seq)
	{
		float round_trip = calculate_round_trip(buffer);
		printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
			   count - sizeof(struct ip), g_ping.hostip, seq, ip_hdr->ip_ttl, round_trip);
		g_ping.received_packets++;
		if (round_trip < g_ping.round_trip_min || g_ping.round_trip_min == -1)
			g_ping.round_trip_min = round_trip;
		if (round_trip >  g_ping.round_trip_max || g_ping.round_trip_max == -1)
			g_ping.round_trip_max = round_trip;
		return 1;
	}
	return 0;
}

void send_request(struct icmphdr *icmp_hdr, unsigned char *packet, int *seq)
{
	bzero(packet, ICMP_PACKET_SIZE);
	// printf("seq=%d\n", *seq);
	icmp_hdr->un.echo.sequence = htons(*seq);
	// printf("packet seq=%d\n", ntohs(icmp_hdr->un.echo.sequence));
	memcpy(packet, icmp_hdr, sizeof(struct icmphdr));
	gettimeofday((struct timeval *)(packet + sizeof(struct icmphdr)), NULL);

	memcpy(packet + sizeof(struct icmphdr) + sizeof(struct timeval), ICMP_PAYLOAD_CHUNK, ICMP_PAYLOAD_CHUNK_SIZE);

	uint16_t checksum = calculate_checksum((uint16_t *)packet, ICMP_PACKET_SIZE);
	//big endian
	packet[2] = checksum & 0xff; // second byte
	packet[3] = checksum >> 8; // first byte

	// printf("\n");
	// printf("ICMP_ECHO\n");
	// printf("icmp_hdr->type=%d\n", icmp_hdr->type);
	// printf("icmp_hdr->un.echo.id=%d\n", ntohs(icmp_hdr->un.echo.id));
	// printf("icmp_hdr->un.echo.sequence=%d \n", ntohs(icmp_hdr->un.echo.sequence));
	// printf("icmp_hdr->checksum=0x%x\n", ntohs(icmp_hdr->checksum));
	// hex_print_data(packet, sizeof(struct icmphdr) + ICMP_PAYLOAD_SIZE);
	// printf("send request\n");
	if (sendto(g_ping.sockfd, packet, ICMP_PACKET_SIZE, 0, g_ping.host->ai_addr, g_ping.host->ai_addrlen) < 0)
	{
		error(EXIT_FAILURE, errno, "sendto");
	}
	g_ping.sent_packets++;
}

int send_echo_requests()
{
	unsigned char packet[ICMP_PACKET_SIZE];

	struct icmphdr icmp_hdr = {
		.type = ICMP_ECHO, // no need htons coz its a byte
		.code = 0, // same
		.un.echo.id = htons(g_ping.self_pid),
	};

	printf("PING %s (%s): %ld data bytes\n", g_ping.hostarg, g_ping.hostip, ICMP_PAYLOAD_SIZE);

	fd_set readfds;
	int select_ret;
	FD_ZERO(&readfds);

	int seq = 0;
	send_request(&icmp_hdr, packet, &seq);
	while (g_ping.loop == true)
	{
		FD_SET(g_ping.sockfd, &readfds);

		struct timeval tv = {1, 0};
		if ((select_ret = select(g_ping.sockfd + 1, &readfds, NULL, NULL, &tv)) < 0)
		{
			if (errno != EINTR)
				error(EXIT_FAILURE, errno, "select");
		}
		else if (select_ret == 0) //timeout
		{
			send_request(&icmp_hdr, packet, &seq);
			seq++;
		}
		else
			get_reply(seq);
	}
	return 0;
}

void ft_ping()
{
	if ((g_ping.sockfd = socket(g_ping.host->ai_family, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		error(EXIT_FAILURE, errno, "socket");
	}

	g_ping.self_pid = getpid();

	struct sigaction action;
	action.sa_handler = signal_handler;
	sigaction(SIGINT, &action, NULL);
	
	send_echo_requests();
	print_stats();
}