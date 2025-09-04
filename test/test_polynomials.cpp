#include <array>

#include "polynomials.hpp"
#include "test_polynomials_tvs.hpp"
#include "test.hpp"

#include <catch2/catch_test_macros.hpp>


TEST_CASE("poly64 load and store", "[polynomial]")
{
    std::array<uint8_t, 8 * POLY_VEC_LEN> tmp = {0};
    for (size_t offset = 0; offset < enc_poly64_vec_xs.size(); offset += 8 * POLY_VEC_LEN)
    {
        poly64 a = poly64::load(enc_poly64_vec_xs.data() + offset);
        a.store(tmp.data());
        REQUIRE(memcmp(tmp.data(), enc_poly64_vec_xs.data() + offset, 8 * POLY_VEC_LEN) == 0);
    }
}

TEST_CASE("poly64 eq", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly64_vec_xs.size(); offset += 8 * POLY_VEC_LEN)
    {
        poly64 a = poly64::load(enc_poly64_vec_xs.data() + offset);
        poly64 b = poly64::load(enc_poly64_vec_ys.data() + offset);
        REQUIRE_POLYVEC_EQ(a, a);
        REQUIRE_POLYVEC_EQ(b, b);
        REQUIRE_POLYVEC_NEQ(a, b);
        REQUIRE_POLYVEC_NEQ(b, a);
    }
}

TEST_CASE("poly64 add", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly64_vec_xs.size(); offset += 8 * POLY_VEC_LEN)
    {
        poly64 a = poly64::load(enc_poly64_vec_xs.data() + offset);
        poly64 b = poly64::load(enc_poly64_vec_ys.data() + offset);
        poly64 sum = poly64::load(enc_poly64_vec_sums.data() + offset);
        REQUIRE_POLYVEC_EQ(a + b, sum);
        REQUIRE_POLYVEC_EQ(b + a, sum);
    }
}

TEST_CASE("poly64 mul", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly64_vec_xs.size(); offset += 8 * POLY_VEC_LEN)
    {
        poly64 a = poly64::load(enc_poly64_vec_xs.data() + offset);
        poly64 b = poly64::load(enc_poly64_vec_ys.data() + offset);
        poly128 unreduced_product =
            poly128::load(enc_poly64_vec_unreduced_products.data() + 2 * offset);
        REQUIRE_POLYVEC_EQ(a * b, unreduced_product);
        REQUIRE_POLYVEC_EQ(b * a, unreduced_product);
    }
}

TEST_CASE("poly64 mul/reduce", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly64_vec_xs.size(); offset += 8 * POLY_VEC_LEN)
    {
        poly64 a = poly64::load(enc_poly64_vec_xs.data() + offset);
        poly64 b = poly64::load(enc_poly64_vec_ys.data() + offset);
        poly64 product = poly64::load(enc_poly64_vec_products.data() + offset);
        REQUIRE_POLYVEC_EQ((a * b).reduce_to<64>(), product);
        REQUIRE_POLYVEC_EQ((b * a).reduce_to<64>(), product);
    }
}

TEST_CASE("poly128 load and store", "[polynomial]")
{
    std::array<uint8_t, 16 * POLY_VEC_LEN> tmp = {0};
    for (size_t offset = 0; offset < enc_poly128_vec_xs.size(); offset += 16 * POLY_VEC_LEN)
    {
        poly128 a = poly128::load(enc_poly128_vec_xs.data() + offset);
        a.store(tmp.data());
        REQUIRE(memcmp(tmp.data(), enc_poly128_vec_xs.data() + offset, 16 * POLY_VEC_LEN) == 0);
    }
}

TEST_CASE("poly128 eq", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly128_vec_xs.size(); offset += 16 * POLY_VEC_LEN)
    {
        poly128 a = poly128::load(enc_poly128_vec_xs.data() + offset);
        poly128 b = poly128::load(enc_poly128_vec_ys.data() + offset);
        REQUIRE_POLYVEC_EQ(a, a);
        REQUIRE_POLYVEC_EQ(b, b);
        REQUIRE_POLYVEC_NEQ(a, b);
        REQUIRE_POLYVEC_NEQ(b, a);
    }
}

TEST_CASE("poly128 add", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly128_vec_xs.size(); offset += 16 * POLY_VEC_LEN)
    {
        poly128 a = poly128::load(enc_poly128_vec_xs.data() + offset);
        poly128 b = poly128::load(enc_poly128_vec_ys.data() + offset);
        poly128 sum = poly128::load(enc_poly128_vec_sums.data() + offset);
        REQUIRE_POLYVEC_EQ(a + b, sum);
        REQUIRE_POLYVEC_EQ(b + a, sum);
    }
}

TEST_CASE("poly128 mul", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly128_vec_xs.size(); offset += 16 * POLY_VEC_LEN)
    {
        poly128 a = poly128::load(enc_poly128_vec_xs.data() + offset);
        poly128 b = poly128::load(enc_poly128_vec_ys.data() + offset);
        poly256 unreduced_product =
            poly256::load(enc_poly128_vec_unreduced_products.data() + 2 * offset);
        REQUIRE_POLYVEC_EQ(a * b, unreduced_product);
        REQUIRE_POLYVEC_EQ(b * a, unreduced_product);
    }
}

TEST_CASE("poly128 mul/reduce", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly128_vec_xs.size(); offset += 16 * POLY_VEC_LEN)
    {
        poly128 a = poly128::load(enc_poly128_vec_xs.data() + offset);
        poly128 b = poly128::load(enc_poly128_vec_ys.data() + offset);
        poly128 product = poly128::load(enc_poly128_vec_products.data() + offset);
        REQUIRE_POLYVEC_EQ((a * b).reduce_to<128>(), product);
        REQUIRE_POLYVEC_EQ((b * a).reduce_to<128>(), product);
    }
}

TEST_CASE("poly192 load and store", "[polynomial]")
{
    std::array<uint8_t, 24 * POLY_VEC_LEN> tmp = {0};
    for (size_t offset = 0; offset < enc_poly192_vec_xs.size(); offset += 24 * POLY_VEC_LEN)
    {
        poly192 a = poly192::load(enc_poly192_vec_xs.data() + offset);
        a.store(tmp.data());
        REQUIRE(memcmp(tmp.data(), enc_poly192_vec_xs.data() + offset, 24 * POLY_VEC_LEN) == 0);
    }
}

TEST_CASE("poly192 eq", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly192_vec_xs.size(); offset += 24 * POLY_VEC_LEN)
    {
        poly192 a = poly192::load(enc_poly192_vec_xs.data() + offset);
        poly192 b = poly192::load(enc_poly192_vec_ys.data() + offset);
        REQUIRE_POLYVEC_EQ(a, a);
        REQUIRE_POLYVEC_EQ(b, b);
        REQUIRE_POLYVEC_NEQ(a, b);
        REQUIRE_POLYVEC_NEQ(b, a);
    }
}

TEST_CASE("poly192 add", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly192_vec_xs.size(); offset += 24 * POLY_VEC_LEN)
    {
        poly192 a = poly192::load(enc_poly192_vec_xs.data() + offset);
        poly192 b = poly192::load(enc_poly192_vec_ys.data() + offset);
        poly192 sum = poly192::load(enc_poly192_vec_sums.data() + offset);
        REQUIRE_POLYVEC_EQ((a + b), sum);
        REQUIRE_POLYVEC_EQ((b + a), sum);
    }
}

TEST_CASE("poly192 mul", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly192_vec_xs.size(); offset += 24 * POLY_VEC_LEN)
    {
        poly192 a = poly192::load(enc_poly192_vec_xs.data() + offset);
        poly192 b = poly192::load(enc_poly192_vec_ys.data() + offset);
        poly384 unreduced_product =
            poly384::load(enc_poly192_vec_unreduced_products.data() + 2 * offset);
        REQUIRE_POLYVEC_EQ((a * b), unreduced_product);
        REQUIRE_POLYVEC_EQ((b * a), unreduced_product);
    }
}

TEST_CASE("poly192 mul/reduce", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly192_vec_xs.size(); offset += 24 * POLY_VEC_LEN)
    {
        poly192 a = poly192::load(enc_poly192_vec_xs.data() + offset);
        poly192 b = poly192::load(enc_poly192_vec_ys.data() + offset);
        poly192 product = poly192::load(enc_poly192_vec_products.data() + offset);
        REQUIRE_POLYVEC_EQ((a * b).reduce_to<192>(), product);
        REQUIRE_POLYVEC_EQ((b * a).reduce_to<192>(), product);
    }
}

TEST_CASE("poly256 load and store", "[polynomial]")
{
    std::array<uint8_t, 32 * POLY_VEC_LEN> tmp = {0};
    for (size_t offset = 0; offset < enc_poly256_vec_xs.size(); offset += 32 * POLY_VEC_LEN)
    {
        poly256 a = poly256::load(enc_poly256_vec_xs.data() + offset);
        a.store(tmp.data());
        REQUIRE(memcmp(tmp.data(), enc_poly256_vec_xs.data() + offset, 32 * POLY_VEC_LEN) == 0);
    }
}

TEST_CASE("poly256 eq", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly256_vec_xs.size(); offset += 32 * POLY_VEC_LEN)
    {
        poly256 a = poly256::load(enc_poly256_vec_xs.data() + offset);
        poly256 b = poly256::load(enc_poly256_vec_ys.data() + offset);
        REQUIRE_POLYVEC_EQ(a, a);
        REQUIRE_POLYVEC_EQ(b, b);
        REQUIRE_POLYVEC_NEQ(a, b);
        REQUIRE_POLYVEC_NEQ(b, a);
    }
}

TEST_CASE("poly256 add", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly256_vec_xs.size(); offset += 32 * POLY_VEC_LEN)
    {
        poly256 a = poly256::load(enc_poly256_vec_xs.data() + offset);
        poly256 b = poly256::load(enc_poly256_vec_ys.data() + offset);
        poly256 sum = poly256::load(enc_poly256_vec_sums.data() + offset);
        REQUIRE_POLYVEC_EQ(a + b, sum);
        REQUIRE_POLYVEC_EQ(b + a, sum);
    }
}

TEST_CASE("poly256 mul", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly256_vec_xs.size(); offset += 32 * POLY_VEC_LEN)
    {
        poly256 a = poly256::load(enc_poly256_vec_xs.data() + offset);
        poly256 b = poly256::load(enc_poly256_vec_ys.data() + offset);
        poly512 unreduced_product =
            poly512::load(enc_poly256_vec_unreduced_products.data() + 2 * offset);
        REQUIRE_POLYVEC_EQ(a * b, unreduced_product);
        REQUIRE_POLYVEC_EQ(b * a, unreduced_product);
    }
}

TEST_CASE("poly256 mul/reduce", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly256_vec_xs.size(); offset += 32 * POLY_VEC_LEN)
    {
        poly256 a = poly256::load(enc_poly256_vec_xs.data() + offset);
        poly256 b = poly256::load(enc_poly256_vec_ys.data() + offset);
        poly256 product = poly256::load(enc_poly256_vec_products.data() + offset);
        REQUIRE_POLYVEC_EQ((a * b).reduce_to<256>(), product);
        REQUIRE_POLYVEC_EQ((b * a).reduce_to<256>(), product);
    }
}

TEST_CASE("poly128 from poly64", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly64_vec_xs.size(); offset += 8 * POLY_VEC_LEN)
    {
        poly64 a = poly64::load(enc_poly64_vec_xs.data() + offset);
        poly128 b = poly128::load(enc_poly64_vec_as_poly128_xs.data() + 2 * offset);
        poly128 a_128 = poly128::from(a);
        REQUIRE_POLYVEC_EQ(a_128, b);
    }
}

TEST_CASE("poly192 from poly128", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly128_vec_xs.size(); offset += 16 * POLY_VEC_LEN)
    {
        poly128 a = poly128::load(enc_poly128_vec_xs.data() + offset);
        poly192 b = poly192::load(enc_poly128_vec_as_poly192_xs.data() + (offset / 16) * 24);
        poly192 a_192 = poly192::from(a);
        REQUIRE_POLYVEC_EQ(a_192, b);
    }
}

TEST_CASE("poly256 from poly128", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly128_vec_xs.size(); offset += 16 * POLY_VEC_LEN)
    {
        poly128 a = poly128::load(enc_poly128_vec_xs.data() + offset);
        poly256 b = poly256::load(enc_poly128_vec_as_poly256_xs.data() + 2 * offset);
        poly256 a_256 = poly256::from(a);
        REQUIRE_POLYVEC_EQ(a_256, b);
    }
}

TEST_CASE("poly256 from poly192", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly192_vec_xs.size(); offset += 24 * POLY_VEC_LEN)
    {
        poly192 a = poly192::load(enc_poly192_vec_xs.data() + offset);
        poly256 b = poly256::load(enc_poly192_vec_as_poly256_xs.data() + (offset / 24) * 32);
        poly256 a_256 = poly256::from(a);
        REQUIRE_POLYVEC_EQ(a_256, b);
    }
}

TEST_CASE("poly384 from poly192", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly192_vec_xs.size(); offset += 24 * POLY_VEC_LEN)
    {
        poly192 a = poly192::load(enc_poly192_vec_xs.data() + offset);
        poly384 b = poly384::load(enc_poly192_vec_as_poly384_xs.data() + 2 * offset);
        poly384 a_384 = poly384::from(a);
        REQUIRE_POLYVEC_EQ(a_384, b);
    }
}

TEST_CASE("poly320 from poly256", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly256_vec_xs.size(); offset += 32 * POLY_VEC_LEN)
    {
        poly256 a = poly256::load(enc_poly256_vec_xs.data() + offset);
        poly320 b = poly320::load(enc_poly256_vec_as_poly320_xs.data() + (offset / 32) * 40);
        poly320 a_320 = poly320::from(a);
        REQUIRE_POLYVEC_EQ(a_320, b);
    }
}

TEST_CASE("poly512 from poly256", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly256_vec_xs.size(); offset += 32 * POLY_VEC_LEN)
    {
        poly256 a = poly256::load(enc_poly256_vec_xs.data() + offset);
        poly512 b = poly512::load(enc_poly256_vec_as_poly512_xs.data() + 2 * offset);
        poly512 a_512 = poly512::from(a);
        REQUIRE_POLYVEC_EQ(a_512, b);
    }
}

TEST_CASE("poly128_from_8_poly1", "[polynomial]")
{
    const auto& inputs = poly128_from_8_poly1_input;
    const auto& outputs = poly128_from_8_poly1_output;

    for (size_t index = 0; index < TEST_VEC_LEN; index += POLY_VEC_LEN)
    {
        poly1 bits[8];
        for (size_t bit = 0; bit < 8; ++bit)
            bits[bit] = poly1::load_offset8(&inputs[index], bit);

        poly128 combined = poly128::from_8_poly1(bits);
        std::array<std::array<uint8_t, 16>, POLY_VEC_LEN> combinedPacked;
        combined.store(combinedPacked[0].data());

        for (size_t k = 0; k < POLY_VEC_LEN; ++k)
            CHECK(combinedPacked[k] == outputs[index + k]);
    }
}

TEST_CASE("poly192_from_8_poly1", "[polynomial]")
{
    const auto& inputs = poly192_from_8_poly1_input;
    const auto& outputs = poly192_from_8_poly1_output;

    for (size_t index = 0; index < TEST_VEC_LEN; index += POLY_VEC_LEN)
    {
        poly1 bits[8];
        for (size_t bit = 0; bit < 8; ++bit)
            bits[bit] = poly1::load_offset8(&inputs[index], bit);

        poly192 combined = poly192::from_8_poly1(bits);
        std::array<std::array<uint8_t, 24>, POLY_VEC_LEN> combinedPacked;
        combined.store(combinedPacked[0].data());

        for (size_t k = 0; k < POLY_VEC_LEN; ++k)
            CHECK(combinedPacked[k] == outputs[index + k]);
    }
}

TEST_CASE("poly256_from_8_poly1", "[polynomial]")
{
    const auto& inputs = poly256_from_8_poly1_input;
    const auto& outputs = poly256_from_8_poly1_output;

    for (size_t index = 0; index < TEST_VEC_LEN; index += POLY_VEC_LEN)
    {
        poly1 bits[8];
        for (size_t bit = 0; bit < 8; ++bit)
            bits[bit] = poly1::load_offset8(&inputs[index], bit);

        poly256 combined = poly256::from_8_poly1(bits);
        std::array<std::array<uint8_t, 32>, POLY_VEC_LEN> combinedPacked;
        combined.store(combinedPacked[0].data());

        for (size_t k = 0; k < POLY_VEC_LEN; ++k)
            CHECK(combinedPacked[k] == outputs[index + k]);
    }
}

TEST_CASE("poly128_from_8_poly128", "[polynomial]")
{
    const auto& inputs = poly128_from_8_poly128_input;
    const auto& outputs = poly128_from_8_poly128_output;

    for (size_t index = 0; index < TEST_VEC_LEN; index += POLY_VEC_LEN)
    {
        poly128 polys[8];
        for (size_t bit = 0; bit < 8; ++bit)
            polys[bit] = poly128::load(inputs.data() + (index * 8 + bit) * 16);

        poly128 combined = poly128::from_8_self(polys);
        std::array<std::array<uint8_t, 16>, POLY_VEC_LEN> combinedPacked;
        combined.store(combinedPacked[0].data());

        for (size_t k = 0; k < POLY_VEC_LEN; ++k)
            CHECK(combinedPacked[k] == outputs[index + k]);
    }
}

TEST_CASE("poly192_from_8_poly192", "[polynomial]")
{
    const auto& inputs = poly192_from_8_poly192_input;
    const auto& outputs = poly192_from_8_poly192_output;

    for (size_t index = 0; index < TEST_VEC_LEN; index += POLY_VEC_LEN)
    {
        poly192 polys[8];
        for (size_t bit = 0; bit < 8; ++bit)
            polys[bit] = poly192::load(inputs.data() + (index * 8 + bit) * 24);

        poly192 combined = poly192::from_8_self(polys);
        std::array<std::array<uint8_t, 24>, POLY_VEC_LEN> combinedPacked;
        combined.store(combinedPacked[0].data());

        for (size_t k = 0; k < POLY_VEC_LEN; ++k)
            CHECK(combinedPacked[k] == outputs[index + k]);
    }
}

TEST_CASE("poly256_from_8_poly256", "[polynomial]")
{
    const auto& inputs = poly256_from_8_poly256_input;
    const auto& outputs = poly256_from_8_poly256_output;

    for (size_t index = 0; index < TEST_VEC_LEN; index += POLY_VEC_LEN)
    {
        poly256 polys[8];
        for (size_t bit = 0; bit < 8; ++bit)
            polys[bit] = poly256::load(inputs.data() + (index * 8 + bit) * 32);

        poly256 combined = poly256::from_8_self(polys);
        std::array<std::array<uint8_t, 32>, POLY_VEC_LEN> combinedPacked;
        combined.store(combinedPacked[0].data());

        for (size_t k = 0; k < POLY_VEC_LEN; ++k)
            CHECK(combinedPacked[k] == outputs[index + k]);
    }
}

TEST_CASE("poly256 shift_left_1", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly256_vec_xs.size(); offset += 32 * POLY_VEC_LEN)
    {
        poly256 a = poly256::load(enc_poly256_vec_xs.data() + offset);
        poly256 shifted = poly256::load(enc_poly256_vec_xs_shifted_left_1.data() + offset);
        REQUIRE_POLYVEC_EQ(a.shift_left_1(), shifted);
    }
}

TEST_CASE("poly256 shift_left_8", "[polynomial]")
{
    for (size_t offset = 0; offset < enc_poly256_vec_xs.size(); offset += 32 * POLY_VEC_LEN)
    {
        poly256 a = poly256::load(enc_poly256_vec_xs.data() + offset);
        poly256 shifted = poly256::load(enc_poly256_vec_xs_shifted_left_8.data() + offset);
        REQUIRE_POLYVEC_EQ(a.shift_left_8(), shifted);
    }
}
