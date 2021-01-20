#include "ttcp.hpp"

#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <inttypes.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace po = boost::program_options;

bool parseCommandLine(int argc, char** argv, Option& opt) {
    po::options_description desc{"Usage ./ttcp [option]"};
    desc.add_options()("help,h", "help")(
        "port,p", po::value<std::uint16_t>(&opt.port)->default_value(5000),
        "tcp port")("length,l",
                    po::value<std::int32_t>(&opt.length)->default_value(65536),
                    "buffer length")(
        "number,n", po::value<std::int32_t>(&opt.number)->default_value(8192),
        "number of buffers")("trans,t", po::value<std::string>(&opt.host),
                             "transmit")("recv,r", "receive")(
        "nodelay,D", "set TCP_NODELAY");

    po::variables_map varmap;
    po::store(po::parse_command_line(argc, argv, desc), varmap);
    po::notify(varmap);

    opt.transmit = varmap.count("trans") > 0;
    opt.receive = varmap.count("recv") > 0;
    opt.nodelay = varmap.count("nodelay") > 0;

    if (varmap.count("help")) {
        std::cout << desc << std::endl;
        return false;
    }

    if (opt.transmit == opt.receive) {
        std::cout << "either -t or -r must be specified" << std::endl;
        return false;
    }

    std::cout << "port = " << opt.port << std::endl;
    if (opt.transmit) {
        std::cout << "buffer length = " << opt.length << std::endl;
        std::cout << "number of buffers = " << opt.number << std::endl;
    } else {
        std::cout << "accepting..." << std::endl;
    }

    return true;
}

struct sockaddr_in resolveOrDie(const char* host, std::uint16_t port) {
    // or struct hostent* host = ::gethostbyname(host);
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    int err = ::inet_pton(AF_INET, host, &addr.sin_addr);
    (void)err;
    assert(err == 1); // success
    return addr;
}

int acceptOrDie(std::uint16_t port) {
    int listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    const int yes = 1;
    if (::setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
        std::perror("setsockopt");
        std::exit(1);
    }

    sockaddr_in addr;
    ::bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) {
        std::perror("bind");
        std::exit(1);
    }

    if (::listen(listenfd, 10)) {
        std::perror("listen");
        std::exit(1);
    }

    sockaddr_in peerAddr;
    ::bzero(&peerAddr, sizeof(peerAddr));
    socklen_t addrlen = 0;

    int sockfd =
        ::accept(listenfd, reinterpret_cast<sockaddr*>(&peerAddr), &addrlen);
    if (sockfd < 0) {
        std::perror("accept");
        std::exit(1);
    }

    ::close(listenfd);
    return sockfd;
}

std::int32_t write_n(int sockfd, const void* buf, std::int32_t length) {
    std::int32_t written = 0;
    while (written < length) {
        ssize_t nw = ::write(sockfd, static_cast<const char*>(buf) + written,
                             length - written);
        if (nw > 0) {
            written += static_cast<std::int32_t>(nw);
        } else if (nw == 0) {
            break; // EOF
        } else if (errno != EINTR) {
            std::perror("write");
            break;
        }
    }

    return written;
}

std::int32_t read_n(int sockfd, void* buf, std::int32_t length) {
    std::int32_t readn = 0;
    while (readn < length) {
        ssize_t nr =
            ::read(sockfd, static_cast<char*>(buf) + readn, length - readn);
        if (nr > 0) {
            readn += static_cast<std::int32_t>(nr);
        } else if (nr == 0) {
            break;
        } else if (errno != EINTR) {
            std::perror("read");
            break;
        }
    }

    return readn;
}

void transmit(const Option& opt) {
    sockaddr_in addr = resolveOrDie(opt.host.c_str(), opt.port);
    std::cout << "connecting to " << inet_ntoa(addr.sin_addr) << ":"
              << ntohs(addr.sin_port) << std::endl;

    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    if (::connect(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) {
        std::perror("socket");
        std::cout << "unable to connect " << opt.host << std::endl;
        ::close(sockfd);
        return;
    }

    const int yes = 1;
    if (opt.nodelay) {
        ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
                     static_cast<const void*>(&yes), sizeof(yes));
    }

    std::cout << "connected!" << std::endl;

    auto start = std::chrono::steady_clock::now();
    SessionMessage sessionMessage{0, 0};
    sessionMessage.number = htonl(opt.number);
    sessionMessage.length = htonl(opt.length);

    if (write_n(sockfd, &sessionMessage, sizeof(sessionMessage)) !=
        sizeof(sessionMessage)) {
        std::perror("write session message");
        std::exit(1);
    }

    const int totalLen = static_cast<int>(sizeof(int32_t) + opt.length);
    PayloadMessage* payload = static_cast<PayloadMessage*>(::malloc(totalLen));
    assert(payload);
    payload->length = htonl(opt.length);
    for (std::int32_t i = 0; i < opt.length; ++i) {
        payload->data[i] = "0123456789ABCDEF"[i % 16];
    }

    double totalmb = 1.0 * opt.length * opt.number / 1024.0 / 1024.0;
    printf("%.3f MiB in total\n", totalmb);
    for (std::int32_t i = 0; i < opt.number; ++i) {
        int nw = write_n(sockfd, payload, totalLen);
        assert(nw == totalLen);

        int ack = 0;
        int nr = read_n(sockfd, &ack, sizeof(ack));
        assert(nr == sizeof(ack));
        ack = ntohl(ack);
        assert(ack == opt.length);
    }

    ::free(payload);
    ::close(sockfd);
    auto finish = std::chrono::steady_clock::now();
    auto d =
        std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    double elapsed = d.count() / 1000.0; // seconds
    printf("%.3f seconds\n%.3f MiB/s\n", elapsed, totalmb / elapsed);
}

void receive(const Option& opt) {
    int sockfd = acceptOrDie(opt.port);
    SessionMessage sessionMessage{0, 0};
    if (read_n(sockfd, &sessionMessage, sizeof(sessionMessage)) !=
        sizeof(sessionMessage)) {
        std::perror("read SessionMessage");
        std::exit(1);
    }

    sessionMessage.number = ntohl(sessionMessage.number);
    sessionMessage.length = ntohl(sessionMessage.length);
    std::cout << "receive number = " << sessionMessage.number << std::endl
              << "receive length = " << sessionMessage.length << std::endl;
    const int totalLen =
        static_cast<int>(sizeof(std::int32_t) + sessionMessage.length);
    PayloadMessage* payload = static_cast<PayloadMessage*>(::malloc(totalLen));
    assert(payload);

    for (std::int32_t i = 0; i < sessionMessage.number; ++i) {
        payload->length = 0;
        if (read_n(sockfd, &payload->length, sizeof(payload->length)) !=
            sizeof(payload->length)) {
            std::perror("read length");
            std::exit(1);
        }

        payload->length = ntohl(payload->length);
        assert(payload->length == sessionMessage.length);

        if (read_n(sockfd, payload->data, payload->length) != payload->length) {
            std::perror("read data");
            std::exit(1);
        }

        std::int32_t ack = htonl(payload->length);
        if (write_n(sockfd, &ack, sizeof(ack)) != sizeof(ack)) {
            std::perror("write ack");
            std::exit(1);
        }
    }

    ::free(payload);
    ::close(sockfd);
}
