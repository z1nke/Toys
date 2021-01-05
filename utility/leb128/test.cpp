#include <gtest/gtest.h>

#include "leb128.hpp"
#include <cstring>
#include <climits>

TEST(ULEB128, EncodeDecode) {
    // 0x04A3 -> 0x09A3(little endian)
    std::uint8_t buf[16] = {0};
    EXPECT_EQ(2, encodeULEB128(0x4A3, buf));
    EXPECT_EQ(0xA3, buf[0]);
    EXPECT_EQ(0x09, buf[1]);

    EXPECT_EQ(0x4A3, decodeULEB128(buf));

    // 10000 -> 0x4E90(little endian)
    memset(buf, 16, sizeof(std::uint8_t));
    EXPECT_EQ(2, encodeULEB128(10000, buf));
    EXPECT_EQ(0x90, buf[0]);
    EXPECT_EQ(0x4E, buf[1]);

    EXPECT_EQ(10000, decodeULEB128(buf));

    // boundary
    // U64_MAX -> 0x01FFFFFFFFFFFFFFFFFF(little endian)
    memset(buf, 16, sizeof(std::uint8_t));
    EXPECT_EQ(10, encodeULEB128(std::numeric_limits<std::uint64_t>::max(), buf));
    EXPECT_EQ(0xFF, buf[0]);
    EXPECT_EQ(0xFF, buf[1]);
    EXPECT_EQ(0xFF, buf[2]);
    EXPECT_EQ(0xFF, buf[3]);
    EXPECT_EQ(0xFF, buf[4]);
    EXPECT_EQ(0xFF, buf[5]);
    EXPECT_EQ(0xFF, buf[6]);
    EXPECT_EQ(0xFF, buf[7]);
    EXPECT_EQ(0xFF, buf[8]);
    EXPECT_EQ(0x01, buf[9]);

    EXPECT_EQ(std::numeric_limits<std::uint64_t>::max(), decodeULEB128(buf));
}

TEST(SLEB128, EncodeDecode) {
    // 0x04A3 -> 0x09A3(little endian)
    std::uint8_t buf[16] = {0};
    EXPECT_EQ(2, encodeSLEB128(0x4A3, buf));
    EXPECT_EQ(0xA3, buf[0]);
    EXPECT_EQ(0x09, buf[1]);

    EXPECT_EQ(0x4A3, decodeSLEB128(buf));

    // 10000 -> 0xCE90(little endian)
    memset(buf, 16, sizeof(std::uint8_t));
    EXPECT_EQ(3, encodeSLEB128(10000, buf));
    EXPECT_EQ(0x90, buf[0]);
    EXPECT_EQ(0xCE, buf[1]);

    EXPECT_EQ(10000, decodeSLEB128(buf));

    // 10000 -> 0x4E90(little endian)
    memset(buf, 16, sizeof(std::uint8_t));

    // boundary
    // I64_MAX -> 0xFFFF FFFF FFFF FFFF FF(little endian)
    memset(buf, 16, sizeof(std::uint8_t));
    EXPECT_EQ(10, encodeSLEB128(std::numeric_limits<std::int64_t>::max(), buf));
    EXPECT_EQ(0xFF, buf[0]);
    EXPECT_EQ(0xFF, buf[1]);
    EXPECT_EQ(0xFF, buf[2]);
    EXPECT_EQ(0xFF, buf[3]);
    EXPECT_EQ(0xFF, buf[4]);
    EXPECT_EQ(0xFF, buf[5]);
    EXPECT_EQ(0xFF, buf[6]);
    EXPECT_EQ(0xFF, buf[7]);
    EXPECT_EQ(0xFF, buf[8]);

    EXPECT_EQ(std::numeric_limits<std::int64_t>::max(), decodeSLEB128(buf));
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}