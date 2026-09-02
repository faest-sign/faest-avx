#include <array>

#include "test.hpp"
#include "util.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("extract_bits", "[utils]")
{
    // Naive implementation to test against
    const auto stupid_extract_bits = [](size_t N, const uint8_t* src, size_t pos) -> uint64_t
    {
        uint64_t output = 0;
        for (size_t i = 0; i < N; ++i, ++pos)
        {
            size_t byte_offset = pos / 8;
            size_t bit_offset = pos % 8;
            uint64_t bit = (src[byte_offset] >> bit_offset) & 1;
            output |= bit << i;
        }
        return output;
    };

    SECTION("single bit")
    {
        const std::array<uint8_t, 1> data = {0b1010'0101};

        REQUIRE(extract_bits<1>(data.data(), 0) == 1);
        REQUIRE(extract_bits<1>(data.data(), 1) == 0);
        REQUIRE(extract_bits<1>(data.data(), 2) == 1);
        REQUIRE(extract_bits<1>(data.data(), 7) == 1);
    }

    SECTION("extract whole byte")
    {
        const std::array<uint8_t, 2> data = {0xA5, 0x3C};

        REQUIRE(extract_bits<8>(data.data(), 0) == 0xA5);
        REQUIRE(extract_bits<8>(data.data(), 8) == 0x3C);
    }

    SECTION("fields entirely within one byte")
    {
        const std::array<uint8_t, 1> data = {0b1101'0110};

        REQUIRE(extract_bits<4>(data.data(), 2) == 0b0101);
        REQUIRE(extract_bits<3>(data.data(), 1) == 0b011);
    }

    SECTION("spans two bytes")
    {
        const std::array<uint8_t, 2> data = {0xCD, 0xAB};

        REQUIRE(extract_bits<8>(data.data(), 4) == 0xBC);
    }

    SECTION("byte-aligned multi-byte")
    {
        const std::array<uint8_t, 4> data = {0xEF, 0xBE, 0xAD, 0xDE};

        REQUIRE(extract_bits<16>(data.data(), 0) == 0xBEEF);
        REQUIRE(extract_bits<16>(data.data(), 8) == 0xADBE);
        REQUIRE(extract_bits<24>(data.data(), 8) == 0xDEADBE);
    }

    SECTION("unaligned multi-byte")
    {
        const std::array<uint8_t, 4> data = {0x12, 0x34, 0x56, 0x78};

        REQUIRE(extract_bits<12>(data.data(), 3) == 0x682);
        REQUIRE(extract_bits<17>(data.data(), 5) == 0xb1a0);
        REQUIRE(extract_bits<20>(data.data(), 7) == 0xac68);
    }

    SECTION("maximum width (56 bits)")
    {
        const std::array<uint8_t, 7> data = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD};

        REQUIRE(extract_bits<56>(data.data(), 0) == 0xCDAB8967452301ULL);
    }

    SECTION("maximum width with offset")
    {
        const std::array<uint8_t, 8> data = {0xFF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD};

        REQUIRE(extract_bits<56>(data.data(), 1) == stupid_extract_bits(56, data.data(), 1));
    }

    SECTION("all zeros and all ones")
    {
        const std::array<uint8_t, 8> zeros = {};
        const std::array<uint8_t, 8> ones = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        REQUIRE(extract_bits<32>(zeros.data(), 5) == 0);
        REQUIRE(extract_bits<56>(ones.data(), 0) == 0x00FFFFFFFFFFFFFFULL);
    }
}

TEST_CASE("insert_bits", "[utils]")
{
    // Naive implementation to test against
    const auto stupid_insert_bits = [](size_t N, uint8_t* dst, size_t pos, uint64_t val)
    {
        for (size_t i = 0; i < N; ++i, ++pos)
        {
            uint64_t bit = (val >> i) & 1;
            size_t byte_offset = pos / 8;
            size_t bit_offset = pos % 8;
            if (bit)
            {
                dst[byte_offset] |= 1 << bit_offset;
            }
            else
            {
                dst[byte_offset] &= ~(1 << bit_offset);
            }
        }
    };

    SECTION("single bit")
    {
        std::array<uint8_t, 1> dst{0};

        insert_bits<1>(dst.data(), 0, 1);
        REQUIRE(+dst[0] == 0b0000'0001);

        dst = {0};
        insert_bits<1>(dst.data(), 7, 1);
        REQUIRE(+dst[0] == 0b1000'0000);

        dst = {0xFF};
        insert_bits<1>(dst.data(), 3, 0);
        REQUIRE(+dst[0] == 0b1111'0111);
    }

    SECTION("whole byte")
    {
        std::array<uint8_t, 2> dst{0xAA, 0x55};

        insert_bits<8>(dst.data(), 0, 0x3C);

        REQUIRE(dst[0] == 0x3C);
        REQUIRE(dst[1] == 0x55);
    }

    SECTION("field entirely within one byte")
    {
        std::array<uint8_t, 1> dst{0xFF};

        insert_bits<4>(dst.data(), 2, 0);
        REQUIRE(dst[0] == 0b1100'0011);
    }

    SECTION("crosses one byte boundary")
    {
        std::array<uint8_t, 2> dst{0xFF, 0xFF};
        const std::array<uint8_t, 2> expected_dst{0xAF, 0xF5};

        insert_bits<8>(dst.data(), 4, 0x5A);
        REQUIRE(dst == expected_dst);
    }

    SECTION("byte-aligned multi-byte write")
    {
        std::array<uint8_t, 4> dst{0xFF, 0xFF, 0xFF, 0xFF};
        const std::array<uint8_t, 4> expected_dst{0xFF, 0x34, 0x12, 0xFF};

        insert_bits<16>(dst.data(), 8, 0x1234);
        REQUIRE(dst == expected_dst);
    }

    SECTION("unaligned multi-byte write")
    {
        std::array<uint8_t, 8> dst{0x13, 0x57, 0x9B, 0xDF, 0x24, 0x68, 0xAC, 0xF0};

        auto expected_dst = dst;
        stupid_insert_bits(20, expected_dst.data(), 5, 0xABCDE);

        insert_bits<20>(dst.data(), 5, 0xABCDE);

        REQUIRE(dst == expected_dst);
    }

    SECTION("maximum width")
    {
        std::array<uint8_t, 8> dst{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        const std::array<uint8_t, 8> expected_dst{0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12, 0xFF};

        insert_bits<56>(dst.data(), 0, 0x123456789ABCDEULL);
        REQUIRE(dst == expected_dst);
    }

    SECTION("maximum width with offset")
    {
        std::array<uint8_t, 9> dst{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x55};
        auto expected = dst;
        constexpr uint64_t value = 0xFEDCBA98765432ULL;
        stupid_insert_bits(56, expected.data(), 3, value);

        insert_bits<56>(dst.data(), 3, value);
        REQUIRE(dst == expected);
    }

    SECTION("writing zero only clears target bits")
    {
        std::array<uint8_t, 4> dst{0xFF, 0xFF, 0xFF, 0xFF};
        const std::array<uint8_t, 4> expected_dst{0xFF, 0x03, 0x00, 0xF8};

        insert_bits<17>(dst.data(), 10, 0);
        REQUIRE(dst == expected_dst);
    }
}
