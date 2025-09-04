#ifndef TEST_POLYNOMIALS_TVS_HPP
#define TEST_POLYNOMIALS_TVS_HPP

#include <array>
#include <cstdint>

constexpr std::size_t TEST_VEC_LEN = 16;

extern const std::array<uint8_t, 8 * TEST_VEC_LEN> enc_poly64_vec_xs;
extern const std::array<uint8_t, 8 * TEST_VEC_LEN> enc_poly64_vec_ys;
extern const std::array<uint8_t, 8 * TEST_VEC_LEN> enc_poly64_vec_sums;
extern const std::array<uint8_t, 2 * 8 * TEST_VEC_LEN> enc_poly64_vec_unreduced_products;
extern const std::array<uint8_t, 8 * TEST_VEC_LEN> enc_poly64_vec_products;
extern const std::array<uint8_t, 16 * TEST_VEC_LEN> enc_poly128_vec_xs;
extern const std::array<uint8_t, 16 * TEST_VEC_LEN> enc_poly128_vec_ys;
extern const std::array<uint8_t, 16 * TEST_VEC_LEN> enc_poly128_vec_sums;
extern const std::array<uint8_t, 2 * 16 * TEST_VEC_LEN> enc_poly128_vec_unreduced_products;
extern const std::array<uint8_t, 16 * TEST_VEC_LEN> enc_poly128_vec_products;
extern const std::array<uint8_t, 24 * TEST_VEC_LEN> enc_poly192_vec_xs;
extern const std::array<uint8_t, 24 * TEST_VEC_LEN> enc_poly192_vec_ys;
extern const std::array<uint8_t, 24 * TEST_VEC_LEN> enc_poly192_vec_sums;
extern const std::array<uint8_t, 2 * 24 * TEST_VEC_LEN> enc_poly192_vec_unreduced_products;
extern const std::array<uint8_t, 24 * TEST_VEC_LEN> enc_poly192_vec_products;
extern const std::array<uint8_t, 32 * TEST_VEC_LEN> enc_poly256_vec_xs;
extern const std::array<uint8_t, 32 * TEST_VEC_LEN> enc_poly256_vec_ys;
extern const std::array<uint8_t, 32 * TEST_VEC_LEN> enc_poly256_vec_sums;
extern const std::array<uint8_t, 2 * 32 * TEST_VEC_LEN> enc_poly256_vec_unreduced_products;
extern const std::array<uint8_t, 32 * TEST_VEC_LEN> enc_poly256_vec_products;
extern const std::array<uint8_t, 32 * TEST_VEC_LEN> enc_poly256_vec_xs_shifted_left_1;
extern const std::array<uint8_t, 32 * TEST_VEC_LEN> enc_poly256_vec_xs_shifted_left_8;
extern const std::array<uint8_t, 16 * TEST_VEC_LEN> enc_poly64_vec_as_poly128_xs;
extern const std::array<uint8_t, 24 * TEST_VEC_LEN> enc_poly128_vec_as_poly192_xs;
extern const std::array<uint8_t, 32 * TEST_VEC_LEN> enc_poly128_vec_as_poly256_xs;
extern const std::array<uint8_t, 32 * TEST_VEC_LEN> enc_poly192_vec_as_poly256_xs;
extern const std::array<uint8_t, 48 * TEST_VEC_LEN> enc_poly192_vec_as_poly384_xs;
extern const std::array<uint8_t, 40 * TEST_VEC_LEN> enc_poly256_vec_as_poly320_xs;
extern const std::array<uint8_t, 64 * TEST_VEC_LEN> enc_poly256_vec_as_poly512_xs;
extern const std::array<uint8_t, TEST_VEC_LEN> poly128_from_8_poly1_input;
extern const std::array<std::array<uint8_t, 16>, TEST_VEC_LEN> poly128_from_8_poly1_output;
extern const std::array<uint8_t, TEST_VEC_LEN> poly192_from_8_poly1_input;
extern const std::array<std::array<uint8_t, 24>, TEST_VEC_LEN> poly192_from_8_poly1_output;
extern const std::array<uint8_t, TEST_VEC_LEN> poly256_from_8_poly1_input;
extern const std::array<std::array<uint8_t, 32>, TEST_VEC_LEN> poly256_from_8_poly1_output;
extern const std::array<uint8_t, 128 * TEST_VEC_LEN> poly128_from_8_poly128_input;
extern const std::array<std::array<uint8_t, 16>, TEST_VEC_LEN> poly128_from_8_poly128_output;
extern const std::array<uint8_t, 192 * TEST_VEC_LEN> poly192_from_8_poly192_input;
extern const std::array<std::array<uint8_t, 24>, TEST_VEC_LEN> poly192_from_8_poly192_output;
extern const std::array<uint8_t, 256 * TEST_VEC_LEN> poly256_from_8_poly256_input;
extern const std::array<std::array<uint8_t, 32>, TEST_VEC_LEN> poly256_from_8_poly256_output;

#endif
