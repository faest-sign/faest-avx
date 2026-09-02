#include <array>
#include <iostream>
#include <type_traits>
#include <vector>

#include "faest_keys.hpp"
#include "owf_proof.hpp"
#include "test.hpp"
#include "test_vole_helpers.inc"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("bench_owf_proof", "[.][bench][owf proof]", ALL_FAEST_INSTANCES)
{
    using P = TestType;
    constexpr auto S = P::secpar_v;

    std::array<uint8_t, FAEST_SECRET_KEY_BYTES<P>> packed_sk;
    std::array<uint8_t, FAEST_PUBLIC_KEY_BYTES<P>> packed_pk;
    test_gen_keypair<P>(packed_pk.data(), packed_sk.data());
    public_key<P> pk;
    secret_key<P> sk;
    faest_unpack_secret_key(&sk, packed_sk.data());
    faest_unpack_public_key(&pk, packed_pk.data());

    const auto delta = sample_delta<S>(P::delta_bits_v);

    BENCHMARK_ADVANCED("owf_constraints_prover")(Catch::Benchmark::Chronometer meter)
    {
        quicksilver_test_state<S, P::OWF_CONSTS::QS_DEGREE> qs_test(
            P::OWF_CONSTS::OWF_NUM_CONSTRAINTS, reinterpret_cast<uint8_t*>(sk.witness.data()),
            P::OWF_CONSTS::WITNESS_BITS, delta);
        auto& qs_state_prover = qs_test.prover_state;
        meter.measure([&] { return owf_constraints<P>(&qs_state_prover, &pk); });
    };

    BENCHMARK_ADVANCED("owf_constraints_verifier")(Catch::Benchmark::Chronometer meter)
    {
        quicksilver_test_state<S, P::OWF_CONSTS::QS_DEGREE> qs_test(
            P::OWF_CONSTS::OWF_NUM_CONSTRAINTS, reinterpret_cast<uint8_t*>(sk.witness.data()),
            P::OWF_CONSTS::WITNESS_BITS, delta);
        auto& qs_state_verifier = qs_test.verifier_state;
        meter.measure([&] { return owf_constraints<P>(&qs_state_verifier, &pk); });
    };
}
