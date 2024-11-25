#include "test.hpp"

#include "gfsmall.hpp"
#include "test_gfsmall_tvs.hpp"
#include <array>
#include <catch2/catch_test_macros.hpp>


TEST_CASE("gf256_compress_gf16_subfield", "[gfsmall]")
{
    for (size_t i = 0; i < GF16_SUBFIELD_ELEMENTS.size(); ++i)
    {
        REQUIRE(gf256_compress_gf16_subfield(GF16_SUBFIELD_ELEMENTS[i]) ==
                GF16_SUBFIELD_ELEMENTS_COMPRESSED[i]);
    }
}

TEST_CASE("gf256_decompress_gf16_subfield", "[gfsmall]")
{
    for (size_t i = 0; i < GF16_SUBFIELD_ELEMENTS.size(); ++i)
    {
        REQUIRE(gf256_decompress_gf16_subfield(GF16_SUBFIELD_ELEMENTS_COMPRESSED[i]) ==
                GF16_SUBFIELD_ELEMENTS[i]);
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

TEST_CASE("gf256_batch_invnorm", "[gfsmall]")
{
    std::array<uint8_t, 256> gf256_values;
    std::array<uint8_t, 128> compressed_invnorms;
    std::iota(gf256_values.begin(), gf256_values.end(), 0);
    gf256_gf16_batch_invnorm(compressed_invnorms.data(), gf256_values.data(), gf256_values.size());
    REQUIRE(compressed_invnorms == COMPRESSED_GF_256_INVNORMS);
}
