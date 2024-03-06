#include "ft_ping.h"

#include <sys/select.h>

void print_struct_data()
{
    printf("\n");
    //print g_ping data structure
    for(struct addrinfo *p = g_ping.host; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
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

uint16_t calculate_checksum(uint16_t *icmp_hdr, int len)
{
    uint32_t sum = 0;

    //addition of 16-bits words
    while (len > 1)
    {
        sum += *icmp_hdr++;
        len -= 2;
    }
    //if length is odd, add the last byte
    if (len == 1)
        sum += *(uint8_t *)icmp_hdr;
    
    //add carry (high 16 bits to low 16 bits) until carry is zero
    while (sum >> 16)
        sum = (sum >> 16) + (sum & 0xffff);

    //return the one's complement of sum
    return (uint16_t)(~sum);
}

void send_packet(int sockfd, char *packet)
{
    if (sendto(sockfd, packet, sizeof(struct icmp), 0, g_ping.host->ai_addr, g_ping.host->ai_addrlen) < 0)
    {
        error(EXIT_FAILURE, errno, "sendto");
    }
}

void received_packet(int sockfd, int seq)
{
    char buffer[1024];
    struct sockaddr_storage src_addr;
    socklen_t src_addr_len = sizeof(src_addr);
    ssize_t count;

    if ((count = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &src_addr_len)) < 0)
    {
        error(EXIT_FAILURE, errno, "recvfrom");
    }

    struct ip *ip_hdr = (struct ip *)buffer;
    struct icmp *icmp_hdr = (struct icmp *)(buffer + (ip_hdr->ip_hl << 2));

    printf("%d %d\n", seq, icmp_hdr->icmp_seq);

    if (seq == icmp_hdr->icmp_seq && icmp_hdr->icmp_id == htons(g_ping.self_pid) && icmp_hdr->icmp_type == ICMP_ECHOREPLY)
    {
        printf("Received packet\n");
    }
    if (icmp_hdr->icmp_type == ICMP_ECHOREPLY &&\
        icmp_hdr->icmp_id == htons(g_ping.self_pid) &&\
        icmp_hdr->icmp_seq == seq)
    {
        printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
               count, g_ping.hostip, seq, ip_hdr->ip_ttl, 0.0);
    }
}

int send_echo_requests(int sockfd)
{
    char packet[sizeof(struct icmp)];

    struct icmp icmp_hdr = {
        .icmp_type = ICMP_ECHO,
        .icmp_code = 0,
        .icmp_cksum = 0,
        .icmp_id = htons(g_ping.self_pid),
        .icmp_seq = 0,
    };
    icmp_hdr.icmp_cksum = calculate_checksum((uint16_t *)&icmp_hdr, sizeof(struct icmp));

    printf("PING %s (%s): 56 data bytes\n", g_ping.hostarg, g_ping.hostip);

    fd_set readfds;
    int ret;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    while (1)
    {
        memcpy(packet, &icmp_hdr, sizeof(struct icmp));

        send_packet(sockfd, packet);
        if ((ret = select(sockfd + 1, &readfds, NULL, NULL, NULL)) < 0)
            error(EXIT_FAILURE, errno, "select");
        else if (ret == 0)
            printf("Request timeout for icmp_seq %d\n", icmp_hdr.icmp_seq);
        else
            received_packet(sockfd, icmp_hdr.icmp_seq);
        sleep(1);

        icmp_hdr.icmp_seq++;
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