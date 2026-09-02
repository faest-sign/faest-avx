#include "test.hpp"

#include "gfsmall.hpp"
#include "test_gfsmall_tvs.hpp"
#include <array>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>

TEST_CASE("gf256_compress_gf16_subfield", "[gfsmall]")
{
    for (size_t i = 0; i < GF16_SUBFIELD_ELEMENTS.size(); ++i)
    {
        REQUIRE(gf256_compress_gf16_subfield(GF16_SUBFIELD_ELEMENTS[i]) ==
                GF16_SUBFIELD_ELEMENTS_COMPRESSED[i]);
    }
}

TEST_CASE("gf256_batch_compress_gf16_subfield", "[gfsmall]")
{
    block128 input;
    memcpy(&input, GF16_SUBFIELD_ELEMENTS.data(), sizeof(input));
    const auto output = gf256_batch_compress_gf16_subfield(input);
    REQUIRE(memcmp(&output, GF16_SUBFIELD_ELEMENTS_COMPRESSED.data(), sizeof(output)) == 0);
}

TEST_CASE("gf256_decompress_gf16_subfield", "[gfsmall]")
{
    for (size_t i = 0; i < GF16_SUBFIELD_ELEMENTS.size(); ++i)
    {
        REQUIRE(gf256_decompress_gf16_subfield(GF16_SUBFIELD_ELEMENTS_COMPRESSED[i]) ==
                GF16_SUBFIELD_ELEMENTS[i]);
    }
}

#if 0 // disabled to save compile time, tested code is currently unused
TEST_CASE("div_x64_by_q", "[gfsmall]")
{
    // m = X^64 // q
    {
        constexpr uint64_t q = 0b100011011;
        constexpr uint64_t expected_m = 0x11a59bcec98e023;
        constexpr auto m = detail::div_x64_by_q<q>();
        REQUIRE(m == expected_m);
    }
    {
        constexpr uint64_t q = 0x520466e7362;
        constexpr uint64_t expected_m = 0x5767c3;
        constexpr auto m = detail::div_x64_by_q<q>();
        REQUIRE(m == expected_m);
    }
    {
        constexpr uint64_t q = 0xf;
        constexpr uint64_t expected_m = 0x3333333333333333;
        constexpr auto m = detail::div_x64_by_q<q>();
        REQUIRE(m == expected_m);
    }
}

TEST_CASE("barret_reduce_64", "[gfsmall]")
{
    // m = p % q
    {
        constexpr uint64_t q = 0b100011011;
        constexpr uint64_t p = 0x7e;
        constexpr uint64_t expected_m = p;
        auto m = barret_reduce_64<q>(p);
        REQUIRE(m == expected_m);
        m = barret_reduce_64_with_x64divq(p, q, detail::div_x64_by_q<q>());
        REQUIRE(m == expected_m);
    }
    {
        constexpr uint64_t q = 0x520466e7362;
        constexpr uint64_t p = 0x12fe45a6094c5a5a;
        constexpr uint64_t expected_m = 0x248f541bb44;
        auto m = barret_reduce_64<q>(p);
        REQUIRE(m == expected_m);
        m = barret_reduce_64_with_x64divq(p, q, detail::div_x64_by_q<q>());
        REQUIRE(m == expected_m);
    }
}
#endif

TEST_CASE("barret_reduce_64_with_x64divq", "[gfsmall]")
{
    {
        constexpr auto q = 0b100011011;
        constexpr auto x64_div_q = 0x11a59bcec98e023;
        constexpr uint64_t p = 0x7e;
        const auto m = barret_reduce_64_with_x64divq(p, q, x64_div_q);
        REQUIRE(m == p);
    }
    {
        constexpr uint64_t q = 0x520466e7362;
        constexpr uint64_t x64_div_q = 0x5767c3;
        constexpr uint64_t p = 0x12fe45a6094c5a5a;
        constexpr uint64_t expected_m = 0x248f541bb44;
        const auto m = barret_reduce_64_with_x64divq(p, q, x64_div_q);
        REQUIRE(m == expected_m);
    }
}


TEST_CASE("gf256_barret_reduce_64", "[gfsmall]")
{
    for (size_t i = 0; i < POLY64S.size(); ++i)
    {
        const auto x = gf256_barret_reduce_64(block128::set_low64(POLY64S[i]));
        const auto y = block128::set_low64(GF_256_REDUCED_POLY64S[i]);
        REQUIRE(memcmp(&x, &y, sizeof(x)) == 0);
    }
}

TEST_CASE("gf256_invnorm", "[gfsmall]")
{
    for (size_t i = 0; i < 256; ++i)
    {
        REQUIRE(gf256_gf16_invnorm(static_cast<uint8_t>(i)) == GF_256_INVNORMS[i]);
    }
}

TEST_CASE("compress_gf16_vector", "[gfsmall]")
{
    block128 input;
    memcpy(&input, GF16_SUBFIELD_ELEMENTS_COMPRESSED.data(), sizeof(input));
    const auto output = compress_gf16_vector(input);
    const auto* out_p = reinterpret_cast<const uint8_t*>(&output);
    for (size_t i = 0; i < sizeof(input) / 2; ++i)
    {
        const uint8_t a = out_p[i] & 0x0f;
        const uint8_t b = out_p[i] >> 4;
        REQUIRE(a == GF16_SUBFIELD_ELEMENTS_COMPRESSED[2 * i]);
        REQUIRE(b == GF16_SUBFIELD_ELEMENTS_COMPRESSED[2 * i + 1]);
    }
    REQUIRE(output.get_high64() == 0);
}

TEST_CASE("gf256_batch_invnorm", "[gfsmall]")
{
    std::array<uint8_t, 256> gf256_values;
    std::array<uint8_t, 128> compressed_invnorms;
    std::iota(gf256_values.begin(), gf256_values.end(), 0);
    gf256_gf16_batch_invnorm<gf256_values.size()>(compressed_invnorms.data(), gf256_values.data());
    REQUIRE(compressed_invnorms == COMPRESSED_GF_256_INVNORMS);

    const auto test = [&]<typename T>(T)
    {
        constexpr auto n = T::value;
        static_assert(n % 2 == 0);
        INFO("n = " << n);
        std::fill(compressed_invnorms.begin(), compressed_invnorms.end(), 0);
        gf256_gf16_batch_invnorm<n>(compressed_invnorms.data(), gf256_values.data());
        CHECK(std::vector(compressed_invnorms.data(), compressed_invnorms.data() + n / 2) ==
              std::vector(COMPRESSED_GF_256_INVNORMS.data(),
                          COMPRESSED_GF_256_INVNORMS.data() + n / 2));
    };

    test(std::integral_constant<size_t, 2>{});
    test(std::integral_constant<size_t, 16>{});
    test(std::integral_constant<size_t, 24>{});
    test(std::integral_constant<size_t, 32>{});
    test(std::integral_constant<size_t, 36>{});
}

TEST_CASE("bench gf256small", "[.][bench][gfsmall]")
{
    const auto bench = [&]<typename T>(T)
    {
        constexpr auto n = T::value;
        static_assert(n % 2 == 0);
        std::array<uint8_t, n> gf256_values;
        std::generate(gf256_values.begin(), gf256_values.end(), rand<uint8_t>);
        std::array<uint8_t, n / 2> compressed_invnorms;

        BENCHMARK(std::format("batch_invnorm - {}", n))
        {
            gf256_gf16_batch_invnorm<n>(compressed_invnorms.data(), gf256_values.data());
        };
    };

    bench(std::integral_constant<size_t, 16>{});
    bench(std::integral_constant<size_t, 24>{});
    bench(std::integral_constant<size_t, 32>{});
}
