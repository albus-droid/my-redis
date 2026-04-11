#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void msg(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static int32_t read_full(int fd, char *buf, ssize_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        assert((ssize_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t write_all(int fd, const char *buf, ssize_t n) {
   while (n > 0) {
       ssize_t rv = write(fd, buf, n);
       if (rv <= 0) {
           return -1;
       }
       assert((ssize_t)rv <= n);
       n -= (size_t)rv;
       buf += rv;
   }
   return 0;
}

const size_t k_max_message = 4096;

static int do_something(int fd) {
    // 4 bytes header
   char rbuf[4 + k_max_message];
   errno = 0;
   int32_t err = read_full(fd, rbuf, 4);
   if (err) {
       msg(errno == 0 ? "EOF" : "read() error");
       return err;
   }

   uint32_t len = 0;
   memcpy(&len, rbuf, 4);
   if (len > k_max_message) {
       msg("too long");
       return -1;
   }

   err = read_full(fd, &rbuf[4], len);
   if (err) {
       msg("read() error");
       return err;
   }

   printf("client says: %.*s\n", len, &rbuf[4]);

   const char reply[] = "world";
   char wbuf[4 + sizeof(reply)];
   len = (uint32_t)strlen(reply);
   memcpy(wbuf, &len, 4);
   memcpy(&wbuf[4], reply, len);
   return write_all(fd, wbuf, 4 + len);
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
