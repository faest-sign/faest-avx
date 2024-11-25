#ifndef TEST_VOLE_COMMIT_TVS_HPP
#define TEST_VOLE_COMMIT_TVS_HPP

#include "constants.hpp"
#include "parameters.hpp"
#include <array>
#include <cstdint>

using namespace faest;

struct tv_128s
{
    using P = v1::faest_128_s;
    const static std::array<uint8_t, P::secpar_bytes> seed;
    const static std::array<uint8_t, (P::tau_v - 1) * P::CONSTS::VOLE_ROWS / 8> corrections;
    const static std::array<uint8_t, P::CONSTS::VOLE_ROWS / 8> u;
    const static std::array<uint8_t, P::secpar_bytes * P::CONSTS::VOLE_ROWS> v;
    const static std::array<uint8_t, P::secpar_bytes * P::CONSTS::VOLE_ROWS> q;
    const static std::array<uint8_t, 2 * P::secpar_bytes> hcom;
};
struct tv_192s
{
    using P = v1::faest_192_s;
    const static std::array<uint8_t, P::secpar_bytes> seed;
    const static std::array<uint8_t, (P::tau_v - 1) * P::CONSTS::VOLE_ROWS / 8> corrections;
    const static std::array<uint8_t, P::CONSTS::VOLE_ROWS / 8> u;
    const static std::array<uint8_t, P::secpar_bytes * P::CONSTS::VOLE_ROWS> v;
    const static std::array<uint8_t, P::secpar_bytes * P::CONSTS::VOLE_ROWS> q;
    const static std::array<uint8_t, 2 * P::secpar_bytes> hcom;
};
struct tv_256s
{
    using P = v1::faest_256_s;
    const static std::array<uint8_t, P::secpar_bytes> seed;
    const static std::array<uint8_t, (P::tau_v - 1) * P::CONSTS::VOLE_ROWS / 8> corrections;
    const static std::array<uint8_t, P::CONSTS::VOLE_ROWS / 8> u;
    const static std::array<uint8_t, P::secpar_bytes * P::CONSTS::VOLE_ROWS> v;
    const static std::array<uint8_t, P::secpar_bytes * P::CONSTS::VOLE_ROWS> q;
    const static std::array<uint8_t, 2 * P::secpar_bytes> hcom;
};

#endif
