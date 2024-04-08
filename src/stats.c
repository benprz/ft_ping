#include "ft_ping.h"

float calculate_round_trip_stddev(float round_trip_avg) {
    float numerator = g_ping.round_trip_squared_sigma - (g_ping.received_packets * pow(round_trip_avg, 2));
    return numerator / g_ping.received_packets;
}

void print_stats() {
    printf("--- %s ping statistics ---\n", g_ping.hostarg);
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n", 
			g_ping.sent_packets,
			g_ping.received_packets,
			(g_ping.sent_packets - g_ping.received_packets) / g_ping.sent_packets * 100);
    
    float round_trip_avg = (g_ping.round_trip_max + g_ping.round_trip_min) / 2;
    float round_trip_stddev = calculate_round_trip_stddev(round_trip_avg);
    printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
            g_ping.round_trip_min,
            round_trip_avg,
            g_ping.round_trip_max,
            round_trip_stddev);
}