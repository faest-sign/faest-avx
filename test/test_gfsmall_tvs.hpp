#ifndef TEST_GFSMALL_TVS_HPP
#define TEST_GFSMALL_TVS_HPP

#include <array>
#include <cstdint>

extern const std::array<uint8_t, 16> GF16_SUBFIELD_ELEMENTS;
extern const std::array<uint8_t, 16> GF16_SUBFIELD_ELEMENTS_COMPRESSED;
extern const std::array<uint64_t, 16> POLY64S;
extern const std::array<uint8_t, 16> GF_256_REDUCED_POLY64S;
extern const std::array<uint8_t, 256> GF_256_INVNORMS;
extern const std::array<uint8_t, 128> COMPRESSED_GF_256_INVNORMS;

extern const std::array<uint8_t, 16> GF256_RAND_XS;
extern const std::array<uint8_t, 16> GF256_RAND_YS;
extern const std::array<uint8_t, 16> GF256_RAND_ZS;
extern const std::array<uint8_t, 16> GF256_RAND_X_INVS;
extern const std::array<uint8_t, 16> GF256_RAND_XY_SUMS;
extern const std::array<uint8_t, 16> GF256_RAND_XY_PRODUCTS;
extern const std::array<uint8_t, 16> GF256_RAND_XYZ_PRODUCTS;

#endif
