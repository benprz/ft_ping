#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <dlfcn.h>

int socket(int domain, int type, int protocol) {
    printf("domain: %d\ntype: %d\nprotocol: %d\n", domain, type, protocol);

    if (type != SOCK_DGRAM) {
        errno = EPERM;
        return -1;
    }

    printf("SOCK_DGRAM type detected\n");
    // Get a pointer to the original socket function
    int (*original_socket)(int, int, int);
    original_socket = dlsym(RTLD_NEXT, "socket");

    if (!original_socket) {
        fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        return -1;
    }

    printf("Calling the real ping...\n");
    // Call the original socket function
    return original_socket(domain, type, protocol);
}
