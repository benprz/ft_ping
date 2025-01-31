#include "ft_ping.h"

#include <stdlib.h>
#include <sys/select.h>

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

int recv_icmp_reply_request(int seq)
{
	unsigned char buffer[1024];
	struct sockaddr_storage src_addr;
	socklen_t src_addr_len = sizeof(src_addr);
	ssize_t recv_bytes;

	if ((recv_bytes = recvfrom(g_ping.sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &src_addr_len)) < 0)
	   return -1;
	struct ip *ip_hdr = (struct ip *)buffer;
	struct icmphdr *icmp_hdr = (struct icmphdr *)(buffer + IP_HEADER_SIZE);

	if (icmp_hdr->type == ICMP_ECHOREPLY &&(ntohs(icmp_hdr->un.echo.id) == g_ping.self_pid))
	{
		g_ping.received_packets++;

		float round_trip = calculate_round_trip(buffer);
		printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
			   recv_bytes - sizeof(struct ip), g_ping.hostip, seq, ip_hdr->ip_ttl, round_trip);
		if (round_trip < g_ping.round_trip_min || g_ping.round_trip_min == -1)
			g_ping.round_trip_min = round_trip;
		if (round_trip >  g_ping.round_trip_max || g_ping.round_trip_max == -1)
			g_ping.round_trip_max = round_trip;
		g_ping.round_trip_sigma += round_trip;
		g_ping.round_trip_squared_sigma += powf(round_trip, 2);
		return 1;
	}
	return 0;
}

void send_icmp_echo_request(struct icmphdr *icmp_hdr, unsigned char *packet, int *seq)
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

	if (sendto(g_ping.sockfd, packet, ICMP_PACKET_SIZE, 0, g_ping.host->ai_addr, g_ping.host->ai_addrlen) < 0)
		error(EXIT_FAILURE, errno, "sending packet");
	g_ping.sent_packets++;
}

void print_header_output() {
	printf("PING %s (%s): %ld data bytes", g_ping.hostarg, g_ping.hostip, ICMP_PAYLOAD_SIZE);
	if (g_ping.verbose)
		printf(", id 0x%04x = %u", g_ping.self_pid, g_ping.self_pid);
	printf("\n");
}

int send_and_recv_requests()
{
	unsigned char packet[ICMP_PACKET_SIZE];
	fd_set readfds;
	int select_ret;
	int seq = 0;

	struct icmphdr icmp_hdr = {
		.type = ICMP_ECHO, // no need htons coz its a byte
		.code = 0, // same
		.un.echo.id = htons(g_ping.self_pid),
	};
	print_header_output();

	FD_ZERO(&readfds);
	send_icmp_echo_request(&icmp_hdr, packet, &seq);
	while (g_ping.loop == true)
	{
		FD_SET(g_ping.sockfd, &readfds);

		struct timeval tv = {1, 0};
		if ((select_ret = select(g_ping.sockfd + 1, &readfds, NULL, NULL, &tv)) < 0)
		{
			if (errno != EINTR)
				error(EXIT_FAILURE, errno, "select failed");
			continue;
		}
		else if (select_ret == 0) //timeout
		{
			send_icmp_echo_request(&icmp_hdr, packet, &seq);
			seq++;
		}
		else
			recv_icmp_reply_request(seq);
	}
	return 0;
}

void ft_ping()
{
	if ((g_ping.sockfd = socket(g_ping.host->ai_family, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
       	if (errno == EPERM || errno == EACCES || errno == EPROTONOSUPPORT)
            fprintf (stderr, "ft_ping: Lacking privilege for icmp socket.\n");
        else
            fprintf (stderr, "ft_ping: %s\n", strerror (errno));
        exit(EXIT_FAILURE);
	}

	int broadcast = true;
	setsockopt(g_ping.sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

	g_ping.self_pid = getpid();

	struct sigaction action = {0};
	action.sa_handler = signal_handler;
	sigaction(SIGINT, &action, NULL);

	send_and_recv_requests();
	print_stats();
}
