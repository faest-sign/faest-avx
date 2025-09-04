#include <array>

#include "block.hpp"
#include "test.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("block eq", "[block]", block128, block192, block256, block384, block512)
{
    using B = TestType;

    const auto in = rand<B>();
    const auto in_eq = in;
    REQUIRE(in_eq == in);

    B in_neq;
    auto* ineq_p = reinterpret_cast<uint8_t*>(&in_neq);
    uint8_t err;

    for (size_t i = 0; i < sizeof(B); ++i)
    {
        in_neq = in;
        do
        {
            err = rand<uint8_t>();
        } while (err == 0);
        ineq_p[i] ^= err;
        REQUIRE(in_neq != in);
    }
}

TEMPLATE_TEST_CASE("block xor", "[block]", block128, block192, block256, block384, block512)
{
    using B = TestType;
    const auto in_1 = rand<B>();
    const auto in_2 = rand<B>();
    const auto out = in_1 ^ in_2;
    const auto out_2 = in_2 ^ in_1;

    const auto* in_1_p = reinterpret_cast<const uint8_t*>(&in_1);
    const auto* in_2_p = reinterpret_cast<const uint8_t*>(&in_2);
    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);
    for (size_t i = 0; i < sizeof(B); ++i)
    {
        REQUIRE(out_p[i] == (in_1_p[i] ^ in_2_p[i]));
    }
    REQUIRE(memcmp(&out, &out_2, sizeof(B)) == 0);
}

TEMPLATE_TEST_CASE("block and", "[block]", block128, block192, block256, block384, block512)
{
    using B = TestType;
    const auto in_1 = rand<B>();
    const auto in_2 = rand<B>();
    const auto out = in_1 & in_2;
    const auto out_2 = in_2 & in_1;

    const auto* in_1_p = reinterpret_cast<const uint8_t*>(&in_1);
    const auto* in_2_p = reinterpret_cast<const uint8_t*>(&in_2);
    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);
    for (size_t i = 0; i < sizeof(B); ++i)
    {
        REQUIRE(out_p[i] == (in_1_p[i] & in_2_p[i]));
    }
    REQUIRE(memcmp(&out, &out_2, sizeof(B)) == 0);
}

TEMPLATE_TEST_CASE("block or", "[block]", block128, block192, block256, block384, block512)
{
    using B = TestType;
    const auto in_1 = rand<B>();
    const auto in_2 = rand<B>();
    const auto out = in_1 | in_2;
    const auto out_2 = in_2 | in_1;

    const auto* in_1_p = reinterpret_cast<const uint8_t*>(&in_1);
    const auto* in_2_p = reinterpret_cast<const uint8_t*>(&in_2);
    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);
    for (size_t i = 0; i < sizeof(B); ++i)
    {
        REQUIRE(out_p[i] == (in_1_p[i] | in_2_p[i]));
    }
    REQUIRE(memcmp(&out, &out_2, sizeof(B)) == 0);
}

TEMPLATE_TEST_CASE("block add32", "[block]", block128, block192, block256, block384, block512)
{
    using B = TestType;
    const auto in_1 = rand<B>();
    const auto in_2 = rand<B>();
    const auto out = in_1.add32(in_2);
    const auto out_2 = in_2.add32(in_1);

    const auto* in_1_p = reinterpret_cast<const uint8_t*>(&in_1);
    const auto* in_2_p = reinterpret_cast<const uint8_t*>(&in_2);
    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);
    uint32_t a, b, c;
    for (size_t i = 0; i < sizeof(B) / sizeof(a); ++i)
    {
        memcpy(&a, in_1_p + i * sizeof(a), sizeof(a));
        memcpy(&b, in_2_p + i * sizeof(a), sizeof(a));
        memcpy(&c, out_p + i * sizeof(a), sizeof(a));
        REQUIRE(c == a + b);
    }
    REQUIRE(memcmp(&out, &out_2, sizeof(B)) == 0);
}

TEMPLATE_TEST_CASE("block set_zero", "[block]", block128, block192, block256, block384, block512)
{
    using B = TestType;
    const auto out = B::set_zero();

    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);
    for (size_t i = 0; i < sizeof(B); ++i)
    {
        REQUIRE(out_p[i] == 0);
    }
}

TEMPLATE_TEST_CASE("block set_all_8", "[block]", block128, block192, block256, block384, block512)
{
    using B = TestType;
    const auto in = rand<uint8_t>();
    const auto out = B::set_all_8(in);

    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);
    for (size_t i = 0; i < sizeof(B); ++i)
    {
        REQUIRE(out_p[i] == in);
    }
}

TEMPLATE_TEST_CASE("block set_low32", "[block]", block128, block192, block256, block384, block512)
{
    using B = TestType;
    const auto in_1 = rand<uint32_t>();
    const auto in_2 = rand<uint32_t>();
    const auto out = B::set_low32(in_1);
    const auto out_2 = B::set_low32(in_1, in_2);

    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);
    const auto* out_2_p = reinterpret_cast<const uint8_t*>(&out_2);
    uint32_t a;

    memcpy(&a, out_p, sizeof(a));
    REQUIRE(a == in_1);

    memcpy(&a, out_2_p, sizeof(a));
    REQUIRE(a == in_1);

    memcpy(&a, out_p + 4, sizeof(a));
    REQUIRE(a == 0);

    memcpy(&a, out_2_p + 4, sizeof(a));
    REQUIRE(a == in_2);

    for (size_t i = 2; i < sizeof(B) / sizeof(a); ++i)
    {
        memcpy(&a, out_p + i * sizeof(a), sizeof(a));
        REQUIRE(a == 0);
        memcpy(&a, out_2_p + i * sizeof(a), sizeof(a));
        REQUIRE(a == 0);
    }
}

TEMPLATE_TEST_CASE("block set64", "[block]", block128)
{
    using B = TestType;
    const auto in_1 = rand<uint32_t>();
    const auto in_2 = rand<uint32_t>();
    const auto out = B::set_64(in_1, in_2);

    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);
    uint64_t a;

    memcpy(&a, out_p, sizeof(a));
    REQUIRE(a == in_1);

    memcpy(&a, out_p + sizeof(a), sizeof(a));
    REQUIRE(a == in_2);
}

TEMPLATE_TEST_CASE("block set_low64", "[block]", block128, block192, block256, block384, block512)
{
    using B = TestType;
    const auto in_1 = rand<uint64_t>();
    const auto out = B::set_low64(in_1);

    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);
    uint64_t a;

    memcpy(&a, out_p, sizeof(a));
    REQUIRE(a == in_1);

    for (size_t i = 1; i < sizeof(B) / sizeof(a); ++i)
    {
        memcpy(&a, out_p + i * sizeof(a), sizeof(a));
        REQUIRE(a == 0);
    }
}

TEMPLATE_TEST_CASE("block get_high64", "[block]", block128)
{
    using B = TestType;
    const auto in = rand<B>();
    const auto out = in.get_high64();

    const auto* in_p = reinterpret_cast<const uint8_t*>(&in);
    uint64_t a;

    memcpy(&a, in_p + sizeof(B) - sizeof(a), sizeof(a));
    REQUIRE(out == a);
}

TEMPLATE_TEST_CASE("block any_zeros", "[block]", block128, block192, block256)
{
    using B = TestType;

    const auto in_no_zero = rand<B>() | B::set_all_8(0x01);
    REQUIRE(in_no_zero.any_zeros() == false);

    B in_zero;
    auto* in_z_p = reinterpret_cast<uint8_t*>(&in_zero);

    for (size_t i = 0; i < sizeof(B); ++i)
    {
        in_zero = in_no_zero;
        in_z_p[i] = 0x00;
        REQUIRE(in_zero.any_zeros());
    }
}

TEMPLATE_TEST_CASE("block byte_reverse", "[block]", block128)
{
    using B = TestType;
    const auto in = rand<B>();
    const auto out = in.byte_reverse();

    const auto* in_p = reinterpret_cast<const uint8_t*>(&in);
    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);

    for (size_t i = 0; i < sizeof(B); ++i)
    {
        REQUIRE(in_p[i] == out_p[sizeof(B) - i - 1]);
    }
}

// clang-format off

const std::array<std::array<uint8_t, 16>, 2> CLMUL_INPUTS = {{
    {0x6e, 0xd9, 0x68, 0xde, 0xd0, 0xbd, 0x44, 0xfc, 0x0b, 0xf5, 0x76, 0xac, 0x1e, 0x43, 0x2a, 0xa9},
    {0x73, 0xaa, 0xc7, 0x2b, 0x33, 0x65, 0x70, 0xfb, 0xa8, 0x02, 0x19, 0x73, 0x61, 0xe2, 0x81, 0xf9},
}};

const std::array<std::array<uint8_t, 16>, 4> CLMUL_OUTPUTS = {{
    {0x12, 0xe7, 0x70, 0xd3, 0xb9, 0x91, 0x8a, 0x50, 0x62, 0x42, 0x8d, 0x15, 0xa5, 0x3b, 0xb6, 0x57},
    {0xb0, 0x8d, 0x7d, 0x4d, 0x48, 0xfc, 0x47, 0xd6, 0x03, 0x7c, 0xe7, 0x69, 0xd1, 0x4d, 0x1f, 0x56},
    {0x0d, 0x02, 0x80, 0x77, 0x65, 0xe3, 0x35, 0xfe, 0x58, 0x22, 0xff, 0x87, 0xc7, 0xb1, 0xef, 0x65},
    {0xb8, 0x9a, 0x1a, 0x3b, 0x87, 0x76, 0x0e, 0x08, 0x6d, 0xa3, 0xbe, 0xba, 0xdd, 0x73, 0xdc, 0x64},
}};

// clang-format on

TEST_CASE("block128 clmul", "[block]")
{
    block128 a, b, c;
    memcpy(&a, CLMUL_INPUTS[0].data(), sizeof(a));
    memcpy(&b, CLMUL_INPUTS[1].data(), sizeof(b));

    c = block128::clmul_ll(a, b);
    REQUIRE(memcmp(&c, CLMUL_OUTPUTS[0].data(), sizeof(c)) == 0);

    c = block128::clmul_lh(a, b);
    REQUIRE(memcmp(&c, CLMUL_OUTPUTS[1].data(), sizeof(c)) == 0);

    c = block128::clmul_hl(a, b);
    REQUIRE(memcmp(&c, CLMUL_OUTPUTS[2].data(), sizeof(c)) == 0);

    c = block128::clmul_hh(a, b);
    REQUIRE(memcmp(&c, CLMUL_OUTPUTS[3].data(), sizeof(c)) == 0);
}

TEMPLATE_TEST_CASE("block shift_left_64", "[block]", block128)
{
    using B = TestType;
    const auto in = rand<B>();
    const auto out = in.shift_left_64();

    const auto* in_p = reinterpret_cast<const uint8_t*>(&in);
    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);

    uint64_t a, b;
    memcpy(&b, out_p, sizeof(b));
    REQUIRE(b == 0);
    for (size_t i = 0; i < sizeof(B) / sizeof(uint64_t) - 1; ++i)
    {
        memcpy(&a, in_p + i * sizeof(a), sizeof(a));
        memcpy(&b, out_p + (i + 1) * sizeof(b), sizeof(b));
        REQUIRE(b == a);
    }
}

TEMPLATE_TEST_CASE("block shift_right_64", "[block]", block128)
{
    using B = TestType;
    const auto in = rand<B>();
    const auto out = in.shift_right_64();

    const auto* in_p = reinterpret_cast<const uint8_t*>(&in);
    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);

    uint64_t a, b;
    memcpy(&b, out_p + sizeof(B) - sizeof(b), sizeof(b));
    REQUIRE(b == 0);
    for (size_t i = 0; i < sizeof(B) / sizeof(uint64_t) - 1; ++i)
    {
        memcpy(&a, in_p + (i + 1) * sizeof(a), sizeof(a));
        memcpy(&b, out_p + i * sizeof(b), sizeof(b));
        REQUIRE(b == a);
    }
}

TEMPLATE_TEST_CASE("block shift_each_16", "[block]", block128, block256)
{
    using B = TestType;
    const auto in = rand<B>();
    const auto test = [&]<int shift>(const std::integral_constant<int, shift>)
    {
        INFO("shift = " << shift);
        const auto out_1 = in.template shift_left_each_16<shift>();
        const auto out_2 = in.template shift_right_each_16<shift>();

        const auto* in_p = reinterpret_cast<const uint8_t*>(&in);
        const auto* out_1_p = reinterpret_cast<const uint8_t*>(&out_1);
        const auto* out_2_p = reinterpret_cast<const uint8_t*>(&out_2);

        uint16_t a, b, c;
        for (size_t i = 0; i < sizeof(B) / sizeof(uint16_t); ++i)
        {
            memcpy(&a, in_p + i * sizeof(a), sizeof(a));
            memcpy(&b, out_1_p + i * sizeof(b), sizeof(b));
            memcpy(&c, out_2_p + i * sizeof(c), sizeof(c));
            REQUIRE(b == ((a << shift) & 0xffff));
            REQUIRE(c == ((a >> shift) & 0xffff));
        }
    };
    test(std::integral_constant<int, 1>{});
    test(std::integral_constant<int, 2>{});
    test(std::integral_constant<int, 3>{});
    test(std::integral_constant<int, 4>{});
    test(std::integral_constant<int, 5>{});
    test(std::integral_constant<int, 6>{});
    test(std::integral_constant<int, 7>{});
    test(std::integral_constant<int, 8>{});
    test(std::integral_constant<int, 9>{});
    test(std::integral_constant<int, 10>{});
    test(std::integral_constant<int, 11>{});
    test(std::integral_constant<int, 12>{});
    test(std::integral_constant<int, 13>{});
    test(std::integral_constant<int, 14>{});
    test(std::integral_constant<int, 15>{});
}

TEMPLATE_TEST_CASE("block broadcast_low64", "[block]", block128)
{
    using B = TestType;
    const auto in = rand<B>();
    const auto out = in.broadcast_low64();
    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);

    uint64_t a, b;
    memcpy(&a, &in, sizeof(a));
    for (size_t i = 0; i < sizeof(B) / sizeof(uint64_t); ++i)
    {
        memcpy(&b, out_p + i * sizeof(b), sizeof(b));
        REQUIRE(b == a);
    }
}

TEST_CASE("block mix_64", "[block]")
{
    const auto in_1 = rand<block128>();
    const auto in_2 = rand<block128>();
    const auto out = block128::mix_64(in_1, in_2);
    const auto* in_1_p = reinterpret_cast<const uint8_t*>(&in_1);
    const auto* in_2_p = reinterpret_cast<const uint8_t*>(&in_2);
    const auto* out_p = reinterpret_cast<const uint8_t*>(&out);

    REQUIRE(memcmp(out_p, in_2_p + 8, 8) == 0);
    REQUIRE(memcmp(out_p + 8, in_1_p, 8) == 0);
}
