#ifndef TEST_VOLE_COMMIT_TVS_V3
#define TEST_VOLE_COMMIT_TVS_V3

#include <array>
#include <cstdint>

#include "constants.hpp"
#include "parameters.hpp"

namespace test_vole_commit_tvs_v3
{

template <typename P> struct vole_commit_v3_tvs
{
    constexpr static std::size_t HASH_LEN = 64;
    const static std::size_t ell;
    const static std::size_t nmask;
    const static std::size_t crtmults;
    const static std::array<uint8_t, P::secpar_bytes> seed;
    constexpr static std::array<uint8_t, 16> iv{};
    const static std::array<uint8_t, 2 * P::secpar_bytes> com;
    const static std::array<uint8_t, P::secpar_bytes> chall;
    const static std::array<uint8_t, HASH_LEN> hashed_barQ;
    const static std::array<uint8_t, HASH_LEN> hashed_barU;
    const static std::array<uint8_t, HASH_LEN> hashed_barV;
    const static std::array<uint8_t, HASH_LEN> hashed_c;
    const static std::array<uint8_t, HASH_LEN> hashed_cmult;
    const static std::array<uint8_t, HASH_LEN> hashed_mQ;
    const static std::array<uint8_t, HASH_LEN> hashed_mV;
    const static std::array<uint8_t, HASH_LEN> hashed_u;
};

#define DECL_VOLE_COMMIT_TVS(P)                                                                    \
    template <> decltype(vole_commit_v3_tvs<P>::ell) vole_commit_v3_tvs<P>::ell;                   \
    template <> decltype(vole_commit_v3_tvs<P>::nmask) vole_commit_v3_tvs<P>::nmask;               \
    template <> decltype(vole_commit_v3_tvs<P>::crtmults) vole_commit_v3_tvs<P>::crtmults;         \
    template <> decltype(vole_commit_v3_tvs<P>::seed) vole_commit_v3_tvs<P>::seed;                 \
    template <> decltype(vole_commit_v3_tvs<P>::com) vole_commit_v3_tvs<P>::com;                   \
    template <> decltype(vole_commit_v3_tvs<P>::chall) vole_commit_v3_tvs<P>::chall;               \
    template <> decltype(vole_commit_v3_tvs<P>::hashed_barQ) vole_commit_v3_tvs<P>::hashed_barQ;   \
    template <> decltype(vole_commit_v3_tvs<P>::hashed_barV) vole_commit_v3_tvs<P>::hashed_barV;   \
    template <> decltype(vole_commit_v3_tvs<P>::hashed_barU) vole_commit_v3_tvs<P>::hashed_barU;   \
    template <> decltype(vole_commit_v3_tvs<P>::hashed_c) vole_commit_v3_tvs<P>::hashed_c;  \
    template <> decltype(vole_commit_v3_tvs<P>::hashed_cmult) vole_commit_v3_tvs<P>::hashed_cmult;  \
    template <> decltype(vole_commit_v3_tvs<P>::hashed_mV) vole_commit_v3_tvs<P>::hashed_mV;       \
    template <> decltype(vole_commit_v3_tvs<P>::hashed_mQ) vole_commit_v3_tvs<P>::hashed_mQ;       \
    template <> decltype(vole_commit_v3_tvs<P>::hashed_u) vole_commit_v3_tvs<P>::hashed_u;

DECL_VOLE_COMMIT_TVS(faest::v3::faest_128_s);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_128_f);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_192_s);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_192_f);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_256_s);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_256_f);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_em_128_s);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_em_128_f);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_em_192_s);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_em_192_f);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_em_256_s);
DECL_VOLE_COMMIT_TVS(faest::v3::faest_em_256_f);

} // namespace test_vole_commit_tvs_v3

#endif
