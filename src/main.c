// inetutils-2.0 (ping -V)
// -v options
// simple ipv4 (address/hostname) as parameters
// manage FQDN without doing DNS resolution in the packet return

#include "ft_ping.h"
#include <argp.h>
#include <assert.h>

const char args_doc[] = "HOST ...";
const char doc[] = "Send ICMP ECHO_REQUEST packets to network hosts."
				   "\vOptions marked with (root only) are available only to "
				   "superuser.";

struct s_ping g_ping = {
	NULL, //hostarg
	{0}, //hostip
	NULL, //host
	false, //verbose

	0, //sockfd
	0, //selfpid
	false, //socket_dgram
	true, //loop

	0, //sent_packets
	0, //received_packets
	-1, //round_trip_min
	-1, //round_trip_max
	0, //round_trip_sigma
	0, //round_trip_squared_sigma
};

int parse_host(char *host)
{
	struct addrinfo hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(host, NULL, &hints, &g_ping.host) != 0)
	{
		error(EXIT_FAILURE, 0, "unknown host");
	}

	g_ping.hostarg = host;

	struct sockaddr_in *ipv4 = (struct sockaddr_in *)g_ping.host->ai_addr;
	void *addr = &(ipv4->sin_addr);
	inet_ntop(g_ping.host->ai_family, addr, g_ping.hostip, INET_ADDRSTRLEN);

	return 0;
}

int parse_options(int key, char *arg, struct argp_state *state)
{
	switch (key)
	{
	case 'v':
		g_ping.verbose = true;
		break;

	case ARGP_KEY_ARG:
		return parse_host(arg);

	case ARGP_KEY_NO_ARGS:
		argp_error(state, "missing host operand");

	/* FALLTHROUGH */
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

int main(int argc, char **argv)
{
    assert(PACKET_SIZE == 84 && "PACKET_SIZE constant should be 84 bytes (IP_HEADER_SIZE + ICMP_PACKET_SIZE)");

	// argp
	struct argp_option options[] = {
		{"verbose", 'v', 0, 0, "Produce verbose output", 0},
		{0}};

	struct argp argp = {options, parse_options, args_doc, doc, 0, 0, 0};
	argp_parse(&argp, argc, argv, 0, NULL, NULL);

	ft_ping();
	return 0;
}
