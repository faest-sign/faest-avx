#ifndef TEST_PRG_TVS_HPP
#define TEST_PRG_TVS_HPP

#include <array>
#include <cstdint>

template <std::size_t secbits> struct prg_tvs;

template <> struct prg_tvs<128>
{
    constexpr static std::size_t n = 15;
    constexpr static std::size_t bs = 16;
    const static std::array<uint8_t, 16> key;
    const static std::array<uint8_t, bs> iv;
    const static uint32_t tweak;
    const static std::array<uint8_t, n * bs> expected_output;
};

template <> struct prg_tvs<192>
{
    constexpr static std::size_t n = 15;
    constexpr static std::size_t bs = 16;
    const static std::array<uint8_t, 24> key;
    const static std::array<uint8_t, bs> iv;
    const static uint32_t tweak;
    const static std::array<uint8_t, n * bs> expected_output;
};

template <> struct prg_tvs<256>
{
    constexpr static std::size_t n = 15;
    constexpr static std::size_t bs = 16;
    const static std::array<uint8_t, 32> key;
    const static std::array<uint8_t, bs> iv;
    const static uint32_t tweak;
    const static std::array<uint8_t, n * bs> expected_output;
};

#endif
