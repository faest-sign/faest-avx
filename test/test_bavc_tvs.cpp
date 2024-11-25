#include <array>
#include <cstdint>

#include "parameters.hpp"
#include "test_bavc_tvs.hpp"
#include "vector_com.hpp"
#include "bavc_tvs_from_ref.hpp"

using namespace faest;

namespace ns_faest_128_s = bavc_tvs_from_ref::FAEST_128S;
namespace ns_faest_128_f = bavc_tvs_from_ref::FAEST_128F;
namespace ns_faest_192_s = bavc_tvs_from_ref::FAEST_192S;
namespace ns_faest_192_f = bavc_tvs_from_ref::FAEST_192F;
namespace ns_faest_256_s = bavc_tvs_from_ref::FAEST_256S;
namespace ns_faest_256_f = bavc_tvs_from_ref::FAEST_256F;
namespace ns_faest_em_128_s = bavc_tvs_from_ref::FAEST_EM_128S;
namespace ns_faest_em_128_f = bavc_tvs_from_ref::FAEST_EM_128F;
namespace ns_faest_em_192_s = bavc_tvs_from_ref::FAEST_EM_192S;
namespace ns_faest_em_192_f = bavc_tvs_from_ref::FAEST_EM_192F;
namespace ns_faest_em_256_s = bavc_tvs_from_ref::FAEST_EM_256S;
namespace ns_faest_em_256_f = bavc_tvs_from_ref::FAEST_EM_256F;

// clang-format off

#define def_tvs(F) \
    template <> constexpr std::array<uint8_t, 2 * v2::F::secpar_bytes> bavc_tvs<v2::F>::h = ns_##F::h; \
    template <> constexpr std::array<uint16_t, v2::F::tau_v> bavc_tvs<v2::F>::i_delta = ns_##F::i_delta; \
    template <> constexpr std::array<uint8_t, 64> bavc_tvs<v2::F>::hashed_k = ns_##F::hashed_k; \
    template <> constexpr std::array<uint8_t, 64> bavc_tvs<v2::F>::hashed_sd = ns_##F::hashed_sd; \
    template <> constexpr std::array<uint8_t, 64> bavc_tvs<v2::F>::hashed_decom_i = ns_##F::hashed_decom_i; \
    template <> constexpr std::array<uint8_t, 64> bavc_tvs<v2::F>::hashed_rec_sd = ns_##F::hashed_rec_sd;

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
