#include <array>

#include "faest_keys.hpp"
#include "owf_proof.hpp"
#include "test.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("owf proof", "[owf proof]", ALL_FAEST_V1_INSTANCES)
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

    const auto delta = rand<block_secpar<S>>();
    quicksilver_test_state<S> qs_test(P::OWF_CONSTS::OWF_NUM_CONSTRAINTS,
                                      reinterpret_cast<uint8_t*>(sk.witness),
                                      P::OWF_CONSTS::WITNESS_BITS, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    owf_constraints<P>(&qs_state_prover, &pk);
    owf_constraints<P>(&qs_state_verifier, &pk);

    auto [check_prover, check_verifier] = qs_test.compute_check();
    REQUIRE(check_prover == check_verifier);
}
