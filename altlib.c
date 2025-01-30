#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <dlfcn.h>

// int socket(int domain, int type, int protocol) {
//     printf("domain: %d\ntype: %d\nprotocol: %d\n", domain, type, protocol);

//     // if (type != SOCK_DGRAM) {
//     //     errno = EPERM;
//     //     return -1;
//     // }

//     // Get a pointer to the original socket function
//     int (*original_socket)(int, int, int);
//     original_socket = dlsym(RTLD_NEXT, "socket");

//     if (!original_socket) {
//         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
//         return -1;
//     }

//     printf("Calling the real ping...\n");
//     // Call the original socket function
//     int fd = original_socket(domain, type, protocol);
//     if (errno) {
//         printf("errno = %d\n", errno);
//         return -1;
//     }
//     return fd;
// }
// ssize_t sendto (int __fd, const void *__buf, size_t __n, int __flags, __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len) {
//     // Get a pointer to the original sendto function
//     ssize_t (*original_sendto)(int, const void *, size_t, int, __CONST_SOCKADDR_ARG, socklen_t);
//     original_sendto = dlsym(RTLD_NEXT, "sendto");

//     if (!original_sendto) {
//         fprintf(stderr, "Error in dlsym: %s\n", dlerror());
//         return -1;
//     }
//     return -1;

//     printf("Calling the real sendto...\n");
//     // Call the original sendto function
//     ssize_t ret = original_sendto(__fd, __buf, __n, __flags, __addr, __addr_len);
//     if (errno) {
//         printf("errno = %d\n", errno);
//         return -1;
//     }
//     return ret;
// }
int select (int __nfds, fd_set *__restrict __readfds,
		   fd_set *__restrict __writefds,
		   fd_set *__restrict __exceptfds,
		   struct timeval *__restrict __timeout) {
	return -1;
}
