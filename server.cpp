#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static int do_something(int fd) {
   char buf[4096] = {};
   ssize_t n = read(fd, buf, sizeof(buf) - 1);
   if (n < 0) {
       msg("read() failed");
       return -1;
   }
   fprintf(stderr, "client says: %.*s\n", (int)n, buf);

   char reply[4096] = "world";
   write(fd, reply, strlen(reply));
   return 0;
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket");
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);

    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));

    if (rv) {
        die("bind");
    }

    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("listen");
    }

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            continue;
        }
        while (true) {
            int32_t err = do_something(client_fd);
            if (err) {
                break;
            }
        }
        close(client_fd);
    }
    return 0;
}
