#pragma once

#include <cstdint>

constexpr std::uint32_t encodeULEB128(std::uint64_t value, std::uint8_t* buf) {
    std::uint8_t* org = buf;
    do {
        uint8_t byte = value & 0x7F; // low order 7 bits of value
        value >>= 7;
        if (value != 0) {
            byte |= 0x80;
        }
        *buf++ = byte;
    } while (value != 0);

    return static_cast<std::uint32_t>(buf - org);
}

constexpr std::uint64_t decodeULEB128(const std::uint8_t* buf,
                                      std::uint32_t* count = nullptr,
                                      const char** error = nullptr) {
    const std::uint8_t* org = buf;
    std::uint64_t value = 0;
    std::uint32_t shift = 0;
    if (error) {
        *error = nullptr;
    }
    do {
        std::uint8_t byte = *buf;
        std::uint64_t slice = byte & 0x7F;
        if (shift >= 64 || slice << shift >> shift != slice) {
            if (error) {
                *error = "ULEB128 too big for uint64";
            }
            return 0;
        }

        value += slice << shift;
        shift += 7;
    } while (*buf++ >= 128);

    if (count) {
        *count = static_cast<std::uint32_t>(buf - org);
    }

    return value;
}

constexpr std::uint32_t encodeSLEB128(std::int64_t value, std::uint8_t* buf) {
    std::uint8_t* org = buf;
    bool more = true;

    while (more) {
        std::uint8_t byte = value & 0x7F; // low order 7 bits of value
        // NOTE: assumes signed shift is arithmetic right shift
        value >>= 7;

        if (((value == 0) && (byte & 0x40) == 0) ||  // positive
            ((value == -1) && (byte & 0x40) != 0)) { // negative
            more = 0;
        }

        if (more) {
            byte |= 0x80;
        }

        *buf++ = byte;
    }

    return static_cast<std::uint32_t>(buf - org);
}

constexpr std::int64_t decodeSLEB128(std::uint8_t* buf,
                                     std::uint32_t* count = nullptr,
                                     const char** error = nullptr) {
    const std::uint8_t* org = buf;
    std::int64_t value = 0;
    std::uint32_t shift = 0;
    if (error) {
        *error = nullptr;
    }

    std::uint8_t byte = 0;

    do {
        byte = *buf++;
        value |= (static_cast<std::uint64_t>(byte & 0x7F) << shift);
        shift += 7;

    } while (byte & 0x80);

    if (shift < sizeof(value) * 8 && (byte & 0x40))
        value |= (~0) << shift;
    if (count)
        *count = (unsigned)(buf - org);
    return value;
}