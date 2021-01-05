#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

const std::uint16_t PORT = 5001;
struct Message {
    std::int64_t request;
    std::int64_t response;
} __attribute__((packed));

static_assert(sizeof(Message) == 16, "size of Message should be 16 bytes");

std::int64_t now() {
    timeval tv{0, 0};
    ::gettimeofday(&tv, nullptr);
    return tv.tv_sec * static_cast<std::int64_t>(1000000) + tv.tv_usec;
}

void runServer() {
    int sockfd = ::socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    assert(sockfd);

    struct sockaddr_in servaddr, cliaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (::bind(sockfd, reinterpret_cast<sockaddr *>(&servaddr),
               sizeof(servaddr))) {
        std::perror("bind");
        std::exit(1);
    }

    while (true) {
        Message msg{0, 0};
        socklen_t addrlen = sizeof(cliaddr);
        ssize_t nr =
            ::recvfrom(sockfd, &msg, sizeof(msg), 0,
                       reinterpret_cast<sockaddr *>(&cliaddr), &addrlen);
        if (nr == sizeof(msg)) {
            msg.response = now();
            ssize_t nw =
                ::sendto(sockfd, &msg, sizeof(msg), 0,
                         reinterpret_cast<const sockaddr *>(&cliaddr), addrlen);
            if (nw < 0) {
                std::perror("sendto message");
            } else if (nw != sizeof(msg)) {
                std::cout << "send message of " << nw << " bytes, expect "
                          << sizeof(msg) << " bytes\n";
            }
        } else if (nr < 0) {
            std::perror("recvfrom message");
        } else {
            std::cout << "recv message of " << nr << " bytes, expect "
                      << sizeof(msg) << " bytes\n";
        }
    }
}

void runClient(const char *host) {
    int sockfd = ::socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    int err = ::inet_pton(AF_INET, host, &servaddr.sin_addr);
    (void)err;
    assert(err == 1);

    // connection of udp
    // see <<unp>> 8.11
    if (::connect(sockfd, reinterpret_cast<sockaddr *>(&servaddr),
                  sizeof(servaddr))) {
        std::perror("connect");
        std::exit(1);
    }

    std::thread t{[sockfd]() {
        while (true) {
            Message msg{0, 0};
            msg.request = now();
            ssize_t nw = ::write(sockfd, &msg, sizeof(msg));
            if (nw < 0) {
                std::perror("write");
            } else if (nw != sizeof(msg)) {
                std::cout << "sent message of " << nw << " bytes, expect "
                          << sizeof(msg) << " bytes\n";
            }

            ::usleep(200 * 1000); // 200 ms
        }
    }};

    while (true) {
        Message msg{0, 0};
        ssize_t nr = ::read(sockfd, &msg, sizeof(msg));
        if (nr == sizeof(msg)) {
            std::int64_t back = now();
            std::int64_t roundtrip = back - msg.request;
            std::int64_t offset = msg.response - (back + msg.request) / 2;
            std::cout << "round trip = " << roundtrip << ", "
                      << "clock offset " << offset << std::endl;
        } else if (nr < 0) {
            std::perror("read");
        } else {
            std::cout << "read message of " << nr << " bytes, expect "
                      << sizeof(msg) << " bytes\n";
        }
    }
}

int main(int argc, char **argv) {
    const char *helpInfo =
        "Usage ./roundtrip [option]:\n-s\tserver\nhost\tclient";
    if (argc != 2) {
        std::cout << helpInfo << std::endl;
        return 0;
    }

    if (strncmp(argv[1], "-s", 2) == 0) {
        runServer();
    } else {
        runClient(argv[1]);
    }

    return 0;
}
