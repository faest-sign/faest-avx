#ifndef TEST_AES_TVS_HPP
#define TEST_AES_TVS_HPP

#include <array>
#include <cstdint>

template <std::size_t secbits> struct aes_tvs;

template <> struct aes_tvs<128>
{
    constexpr static std::size_t n = 4;
    constexpr static std::size_t bs = 16;
    const static std::array<std::array<uint8_t, bs>, n + 1> AFTER_ADD_ROUND_KEY;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_SBOX_INV;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_SBOX;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_SHIFT_ROWS;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_MIX_COLUMNS;
};

template <> struct aes_tvs<192>
{
    constexpr static std::size_t n = 4;
    constexpr static std::size_t bs = 16;
    const static std::array<std::array<uint8_t, bs>, n + 1> AFTER_ADD_ROUND_KEY;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_SBOX_INV;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_SBOX;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_SHIFT_ROWS;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_MIX_COLUMNS;
};

template <> struct aes_tvs<256>
{
    constexpr static std::size_t n = 4;
    constexpr static std::size_t bs = 16;
    const static std::array<std::array<uint8_t, bs>, n + 1> AFTER_ADD_ROUND_KEY;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_SBOX_INV;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_SBOX;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_SHIFT_ROWS;
    const static std::array<std::array<uint8_t, bs>, n> AFTER_MIX_COLUMNS;
};

#endif
