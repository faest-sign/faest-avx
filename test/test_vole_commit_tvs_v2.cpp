#include <array>
#include <cstdint>

#include "test_vole_commit_tvs_v2.hpp"
#include "vole_tvs_from_ref.hpp"

using namespace faest;

namespace ns_faest_128_s = vole_tvs_from_ref::FAEST_128S;
namespace ns_faest_128_f = vole_tvs_from_ref::FAEST_128F;
namespace ns_faest_192_s = vole_tvs_from_ref::FAEST_192S;
namespace ns_faest_192_f = vole_tvs_from_ref::FAEST_192F;
namespace ns_faest_256_s = vole_tvs_from_ref::FAEST_256S;
namespace ns_faest_256_f = vole_tvs_from_ref::FAEST_256F;
namespace ns_faest_em_128_s = vole_tvs_from_ref::FAEST_EM_128S;
namespace ns_faest_em_128_f = vole_tvs_from_ref::FAEST_EM_128F;
namespace ns_faest_em_192_s = vole_tvs_from_ref::FAEST_EM_192S;
namespace ns_faest_em_192_f = vole_tvs_from_ref::FAEST_EM_192F;
namespace ns_faest_em_256_s = vole_tvs_from_ref::FAEST_EM_256S;
namespace ns_faest_em_256_f = vole_tvs_from_ref::FAEST_EM_256F;

// clang-format off

#define def_tvs(F) \
    template <> constexpr std::array<uint8_t, 2 * v2::F::secpar_bytes> vole_commit_tvs<v2::F>::h = ns_##F::h; \
    template <> constexpr std::array<uint8_t, v2::F::secpar_bytes> vole_commit_tvs<v2::F>::chall = ns_##F::chall; \
    template <> constexpr std::array<uint8_t, 64> vole_commit_tvs<v2::F>::hashed_u = ns_##F::hashed_u; \
    template <> constexpr std::array<uint8_t, 64> vole_commit_tvs<v2::F>::hashed_c = ns_##F::hashed_c; \
    template <> constexpr std::array<uint8_t, 64> vole_commit_tvs<v2::F>::hashed_v = ns_##F::hashed_v; \
    template <> constexpr std::array<uint8_t, 64> vole_commit_tvs<v2::F>::hashed_q = ns_##F::hashed_q;

    // template <> constexpr std::array<uint8_t, v2::F::CONSTS::VOLE_ROWS / 8> vole_commit_tvs<v2::F>::u = ns_##F::u;
    // template <> constexpr std::array<uint8_t, (v2::F::tau_v - 1) * v2::F::CONSTS::VOLE_ROWS / 8> vole_commit_tvs<v2::F>::c = ns_##F::c;
    // template <> constexpr std::array<uint8_t, v2::F::secpar_bits * v2::F::CONSTS::VOLE_ROWS / 8> vole_commit_tvs<v2::F>::v = ns_##F::v;
    // template <> constexpr std::array<uint8_t, v2::F::secpar_bits * v2::F::CONSTS::VOLE_ROWS / 8> vole_commit_tvs<v2::F>::q = ns_##F::q;

// clang-format on

def_tvs(faest_128_s)
def_tvs(faest_128_f)
def_tvs(faest_192_s)
def_tvs(faest_192_f)
def_tvs(faest_256_s)
def_tvs(faest_256_f)
def_tvs(faest_em_128_s)
def_tvs(faest_em_128_f)
def_tvs(faest_em_192_s)
def_tvs(faest_em_192_f)
def_tvs(faest_em_256_s)
def_tvs(faest_em_256_f)
