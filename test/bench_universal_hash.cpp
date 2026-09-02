#include <algorithm>
#include <array>
#include <vector>

#include "constants.hpp"
// #include "kos_vole_check.hpp"
#include "parameters.hpp"
#include "prgs.hpp"
#include "test.hpp"
#include "vector_com.hpp"
#include "vole_commit.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "test_vole_helpers.inc"

constexpr auto bench_for = []<size_t... Ns, typename F>(F&& f)
{ (f.template operator()<Ns>(), ...); };

TEMPLATE_TEST_CASE("bench_uhasher", "[.][bench][universal hash]", v3::faest_128_f, v3::faest_192_f,
                   v3::faest_256_f)
{
    using P = TestType;
    using CP = CONSTANTS<P>;
    using OC = P::OWF_CONSTS;
    constexpr auto S = P::secpar_v;

    auto data_e = vole_commit_data<P, false>::alloc();
    REQUIRE(data_e);
    auto data = std::move(*data_e);

    std::generate(data.u().begin(), data.u().end(), [] { return rand<vole_block>(); });
    std::generate(data.v().begin(), data.v().end(), [] { return rand<block_secpar<S>>(); });
    std::generate(data.u_mask().begin(), data.u_mask().end(),
                  [] { return rand<block_secpar<S>>(); });
    std::generate(data.v_mask().begin(), data.v_mask().end(),
                  [] { return rand<block_secpar<S>>(); });

    constexpr auto n_hash = CP::VOLE_CORRECTION_ROWS;
    constexpr auto n_hash_extra = OC::QS_DEGREE - 1;

    const auto bench_uhasher_gfsecpar = [&]<bool BITS, size_t PKP>
    {
        const auto key_block = rand<block_secpar<S>>();
        const auto key = poly_secpar<S>::load(&key_block);
        uhasher_gfsecpar<S, PKP> uhasher;
        const auto* u = data.u_bytes().data();
        const auto* u_mask = data.u_mask().data();
        const auto* v = data.u_bytes().data();
        const auto title = std::format("uhasher_gfsecpar{}<PKP={}>", (BITS ? "_bits" : ""), PKP);
        BENCHMARK(title.c_str())
        {
            const auto pkey = uhasher.preprocess_key(key);
            uhasher.init(n_hash + n_hash_extra);
            if constexpr (BITS)
            {
                for (size_t i = 0; i < CP::VOLE_CORRECTION_ROWS / 8; ++i)
                    uhasher.update_byte(pkey, u[i]);
                    // for (size_t j = 0; j < 8; ++j)
                    //     uhasher.update(pkey, poly_secpar<S>::from(poly1::load(u[i], j)));
            }
            else
            {
                for (size_t i = 0; i < CP::VOLE_CORRECTION_ROWS; ++i)
                    uhasher.update(pkey, v[i]);
            }
            for (size_t i = 0; i < n_hash_extra; ++i)
                uhasher.update(pkey, poly_secpar<S>::from_block(u_mask[i]));
            return uhasher.finalize();
        };
    };

    const auto bench_uhasher_gfsecpar_64 = [&]<bool BITS>
    {
        const auto key_block = rand<uint64_t>();
        const auto key = poly64::load(&key_block);
        uhasher_gfsecpar_64<S> uhasher;
        const auto* u = data.u_bytes().data();
        const auto* u_mask = data.u_mask().data();
        const auto* v = data.u_bytes().data();
        const auto title = std::format("uhasher_gfsecpar64{}", (BITS ? "_bits" : ""));
        BENCHMARK(title.c_str())
        {
            const auto pkey = uhasher.preprocess_key(key);
            uhasher.init(n_hash + n_hash_extra);
            if constexpr (BITS)
            {
                for (size_t i = 0; i < CP::VOLE_CORRECTION_ROWS / 8; ++i)
                    uhasher.update_byte(pkey, u[i]);
            }
            else
            {
                for (size_t i = 0; i < CP::VOLE_CORRECTION_ROWS; ++i)
                    uhasher.update(pkey, v[i]);
            }
            for (size_t i = 0; i < n_hash_extra; ++i)
                uhasher.update(pkey, poly_secpar<S>::from_block(u_mask[i]));
            return uhasher.finalize();
        };
    };

    bench_for.template operator()<1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16>(
        [&]<size_t PKP>() { bench_uhasher_gfsecpar.template operator()<false, PKP>(); });
    bench_for.template operator()<1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16>(
        [&]<size_t PKP>() { bench_uhasher_gfsecpar.template operator()<true, PKP>(); });
    bench_for.template operator()<false, true>(
        [&]<size_t BITS>() { bench_uhasher_gfsecpar_64.template operator()<BITS>(); });
}
