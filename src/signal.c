#include "ft_ping.h"

void signal_handler(int signal) {
    printf("SIGINT\n");
    if (signal == SIGINT) {
        g_ping.loop = false;
    }
}