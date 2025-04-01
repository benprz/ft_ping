#include "ft_ping.h"

void print_header_output() {
	printf("PING %s (%s): %ld data bytes", g_ping.hostarg, g_ping.hostip, ICMP_PAYLOAD_SIZE);
	if (g_ping.verbose)
		printf(", id 0x%04x = %u", g_ping.self_pid, g_ping.self_pid);
	printf("\n");
}

// The checksum field is the 16 bit one's complement of the one's complement sum of all 16 bit words in the header.
// The one's complement of a binary number is the value obtained by inverting (flipping) all the bits in the binary representation of the number.
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
	// to reduce a 32 bits number into a 16 bits number
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
