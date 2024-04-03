#include "ft_ping.h"

void signal_handler(int signal) {
    if (signal == SIGINT) {
        g_ping.loop = false;
    }
}