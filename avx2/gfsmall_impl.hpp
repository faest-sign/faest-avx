#ifndef GFSMALL_IMPL_HPP
#define GFSMALL_IMPL_HPP

#include "block.hpp"
#include <cassert>
#include <cstdint>

#include <cstring>

namespace faest
{

// Reduction of a polynomial of degree at most 63 modulo the GF(256) polynomial.
inline block128 gf256_barret_reduce_64(block128 x)
{
    // FAEST_ASSERT that the upper 64 bits are zero.
    FAEST_ASSERT(_mm_extract_epi64(x.data, 1) == 0);
    // The modulus polynomial.
    const auto q8 = block128::set_low64(0b100011011);
    // X^64 // q8
    const auto m = block128::set_low64(0x11a59bcec98e023);
    return x ^ block128::clmul_hl(block128::clmul_ll(x, m), q8);
}

inline uint8_t gf256_compress_gf16_subfield(uint8_t x)
{
    // 0, 6, 7, 2
    uint8_t y = 0;
    y |= x & 1;
    y |= (x >> 5) & 2;
    y |= (x >> 5) & 4;
    y |= (x << 1) & 8;
    return y;
}

inline uint8_t gf256_decompress_gf16_subfield(uint8_t x)
{
    // basis: [1, W^6 + W^4, W^7 + W^5 + W^4, W^3 + W^2]
    uint8_t y = 0;
    y |= x & 0b0001;
    y |= (x & 0b1000) >> 1;
    y |= x & 0b1000;
    y |= ((x & 0b0010) << 3) ^ ((x & 0b0100) << 2);
    y |= (x & 0b0100) << 3;
    y |= (x & 0b0010) << 5;
    y |= (x & 0b0100) << 5;
    return y;
}

inline uint8_t gf256_gf16_invnorm(uint8_t in)
{
    const auto x = block128::set_low32(in);                                //  8
    const auto x2 = block128::clmul_ll(x, x);                              // 16
    const auto x4 = block128::clmul_ll(x2, x2);                            // 32
    const auto x8 = gf256_barret_reduce_64(block128::clmul_ll(x4, x4));    //  8
    const auto x16 = block128::clmul_ll(x8, x8);                           // 16
    const auto x32 = block128::clmul_ll(x16, x16);                         // 32
    const auto x64 = gf256_barret_reduce_64(block128::clmul_ll(x32, x32)); //  8
    const auto x128 = block128::clmul_ll(x64, x64);                        // 16
    const auto x36 = gf256_barret_reduce_64(block128::clmul_ll(x4, x32));  //  8
    const auto x10 = block128::clmul_ll(x2, x8);                           // 24
    const auto x191 = block128::clmul_ll(x64, x128);                       // 24
    const auto x46 = block128::clmul_ll(x36, x10);                         // 32
    const auto x238 = block128::clmul_ll(x46, x191);                       // 56
    const auto y = gf256_barret_reduce_64(x238);
    uint8_t out;
    memcpy(&out, &y, 1);
    return out;
}

inline void gf256_gf16_batch_invnorm(uint8_t* out, const uint8_t* in, size_t n)
{
    FAEST_ASSERT(n % 2 == 0);
    for (size_t i = 0; i < n >> 1; ++i)
    {
        out[i] = gf256_compress_gf16_subfield(gf256_gf16_invnorm(in[2 * i])) |
                 (gf256_compress_gf16_subfield(gf256_gf16_invnorm(in[2 * i + 1])) << 4);
    }
}

} // namespace faest

#endif
