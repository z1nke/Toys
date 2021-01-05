#pragma once

#include <cstdint>
#include <string>

struct Option {
    std::uint16_t port;
    std::int32_t length;
    std::int32_t number;
    bool transmit;
    bool receive;
    bool nodelay;
    std::string host;

    Option()
        : port(0), length(0), number(0), transmit(false), receive(false),
          nodelay(0) {}
};

bool parseCommandLine(int argc, char **argv, Option &opt);
struct sockaddr_in resolveOrDie(const char *host, std::uint16_t port);

struct SessionMessage {
    std::int32_t number;
    std::int32_t length;
} __attribute__((packed));

struct PayloadMessage {
    std::int32_t length;
    char data[0]; // flexible array
};

void transmit(const Option &opt);
void receive(const Option &opt);
