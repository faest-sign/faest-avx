#ifndef TEST_BAVC_TVS_HPP
#define TEST_BAVC_TVS_HPP

#include <array>
#include <cstdint>

template <typename P> struct bavc_tvs
{
    constexpr static std::array<uint8_t, 16> iv{};
    const static std::array<uint8_t, 32> seed;
    const static std::array<uint8_t, 2 * P::secpar_bytes> h;
    const static std::array<uint8_t, 64> hashed_k;
    const static std::array<uint8_t, 64> hashed_sd;
    const static std::array<uint16_t, P::tau_v> i_delta;
    const static std::array<uint8_t, 64> hashed_decom_i;
    const static std::array<uint8_t, 64> hashed_rec_sd;
};

template <typename P>
constexpr std::array<uint8_t, 32> bavc_tvs<P>::seed = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
};

#endif
