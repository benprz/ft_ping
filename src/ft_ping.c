#include "ft_ping.h"

#include <stdint.h>
#include <stdlib.h>
#include <sys/select.h>

void recv_icmp_reply_request(int seq)
{
	unsigned char buffer[1024];
	struct sockaddr_storage src_addr;
	socklen_t src_addr_len = sizeof(src_addr);
	ssize_t recv_bytes;

	if ((recv_bytes = recvfrom(g_ping.sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &src_addr_len)) < 0)
	   return ;
	struct ip *ip_hdr = (struct ip *)buffer;
	struct icmphdr *icmp_hdr = (struct icmphdr *)(buffer + IP_HEADER_SIZE);

	if (icmp_hdr->type == ICMP_ECHOREPLY && ntohs(icmp_hdr->un.echo.id) == g_ping.self_pid) {
    	uint16_t verify_checksum = calculate_checksum((uint16_t *)icmp_hdr, ICMP_PACKET_SIZE);
		if (verify_checksum != 0x0000) { // the calculated checksum including the checksum bytes (2) yields a value of 0
			fprintf(stderr, "checksum mismatch from %s\n", g_ping.hostip);
			return ;
		}

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
	}
}

void send_icmp_echo_request(struct icmphdr *icmp_hdr, unsigned char *packet, int *seq)
{
	bzero(packet, ICMP_PACKET_SIZE);
	icmp_hdr->un.echo.sequence = htons(*seq);
	memcpy(packet, icmp_hdr, sizeof(struct icmphdr));
	gettimeofday((struct timeval *)(packet + sizeof(struct icmphdr)), NULL);
	memcpy(packet + sizeof(struct icmphdr) + sizeof(struct timeval), ICMP_PAYLOAD_CHUNK, ICMP_PAYLOAD_CHUNK_SIZE);

	uint16_t checksum = calculate_checksum((uint16_t *)packet, ICMP_PACKET_SIZE);
	packet[2] = checksum & 0xff; // big endian: second byte
	packet[3] = checksum >> 8; // big endian: first byte

	if (sendto(g_ping.sockfd, packet, ICMP_PACKET_SIZE, 0, g_ping.host->ai_addr, g_ping.host->ai_addrlen) < 0)
		error(EXIT_FAILURE, errno, "sending packet");
	g_ping.sent_packets++;
}


int send_and_recv_requests()
{
	unsigned char packet[ICMP_PACKET_SIZE];
	fd_set readfds;
	int select_ret;
	int seq = 0;

	struct icmphdr icmp_hdr = {
		.type = ICMP_ECHO,
		.code = 0,
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
