#include "ft_ping.h"

void print_stats() {
    printf("--- %s ping statistics ---\n", g_ping.hostarg);
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n", 
			g_ping.sent_packets,
			g_ping.received_packets,
			(g_ping.sent_packets - g_ping.received_packets) / g_ping.sent_packets * 100);
    printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", g_ping.round_trip_min, 0.1234, g_ping.round_trip_max, 0.1234);
}