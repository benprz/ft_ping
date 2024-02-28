#include "ft_ping.h"

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

void create_socket()
{
    int sockfd;

    if ((sockfd = socket(g_ping.host->ai_family, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        error(EXIT_FAILURE, errno, "socket");
    }

    struct icmp icmp_hdr = {
        .icmp_type = ICMP_ECHO,
        .icmp_code = 0,
        .icmp_cksum = 0,
        .icmp_id = 0,
        .icmp_seq = 0,
    };

    char packet[sizeof(struct iphdr) + sizeof(struct icmp)];

    memset(packet, sizeof());
}

int ft_ping()
{
    print_struct_data();

    create_socket();
    return 0;
}