#include <array>
#include <vector>

#include "constants.hpp"
#include "kos_vole_check.hpp"
#include "parameters.hpp"
#include "prgs.hpp"
#include "test.hpp"
#include "vector_com.hpp"
#include "vole_commit.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "test_vole_helpers.inc"


TEMPLATE_TEST_CASE("bench_kos_vole_check", "[.][bench][kos vole check]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;
    using CP = CONSTANTS<P>;
    constexpr auto S = P::secpar_v;

    auto vole_com_data_sender_e = vole_commit_data<P, false>::alloc();
    auto vole_com_data_receiver_e = vole_commit_data<P, true>::alloc();
    REQUIRE(vole_com_data_sender_e);
    REQUIRE(vole_com_data_receiver_e);
    auto vole_com_data_sender = std::move(*vole_com_data_sender_e);
    auto vole_com_data_receiver = std::move(*vole_com_data_receiver_e);
    std::array<uint8_t, CP::VOLE_CHECK::CHALLENGE_BYTES> chal1;
    std::generate(chal1.begin(), chal1.end(), [] { return rand(); });
    std::array<uint8_t, CP::VOLE_CHECK::PROOF_BYTES> check_proof_sender;
    std::array<uint8_t, CP::VOLE_CHECK::PROOF_BYTES> check_proof_receiver;
    const auto delta = sample_delta<S>(P::delta_bits_v);

    hash_state hasher;
    hasher.init(S);

    BENCHMARK("kos_vole_check_sender")
    {
        kos_vole_check_sender<P>(
            vole_com_data_sender.u_bytes().template subspan<0, CP::VOLE_CORRECTION_BYTES>(),
            vole_com_data_sender.v().template subspan<0, CP::VOLE_CORRECTION_ROWS>(),
            vole_com_data_sender.u_mask(), vole_com_data_sender.v_mask(), chal1, check_proof_sender,
            hasher);
    };

    BENCHMARK("kos_vole_check_receiver")
    {
        kos_vole_check_receiver<P>(
            delta, vole_com_data_receiver.q().template subspan<0, CP::VOLE_CORRECTION_ROWS>(),
            vole_com_data_receiver.q_mask(), chal1, check_proof_receiver, hasher);
    };
}
