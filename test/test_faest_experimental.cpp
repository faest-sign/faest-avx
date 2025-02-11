#include <array>

#include "all.inc"
#include "parameters.hpp"
#include "test.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

using test_params_1 =
    parameter_set<secpar::s128, 16, owf::aes_ecb_with_zero_sboxes, prg::aes_ctr, prg::aes_ctr,
                  leaf_hash::aes_ctr_stat_bind, 8, {bavc::one_tree, 110}>;
using test_params_2 =
    parameter_set<secpar::s128, 16, owf::aes_em_with_zero_sboxes, prg::aes_ctr, prg::aes_ctr,
                  leaf_hash::aes_ctr_stat_bind, 8, {bavc::one_tree, 112}>;
using test_params_3 =
    parameter_set<secpar::s192, 24, owf::aes_ecb_with_zero_sboxes, prg::aes_ctr, prg::aes_ctr,
                  leaf_hash::aes_ctr_stat_bind, 8, {bavc::one_tree, 163}>;
using test_params_4 =
    parameter_set<secpar::s256, 32, owf::aes_em_with_zero_sboxes, prg::aes_ctr, prg::aes_ctr,
                  leaf_hash::aes_ctr_stat_bind, 8, {bavc::one_tree, 234}>;

#define ALL_FAEST_EXP_INSTANCES test_params_1, test_params_2, test_params_3, test_params_4

TEMPLATE_TEST_CASE("keygen/sign/verify", "[faest-experimental]", ALL_FAEST_EXP_INSTANCES)
{
    using P = TestType;
    std::array<uint8_t, FAEST_SECRET_KEY_BYTES<P>> packed_sk;
    std::array<uint8_t, FAEST_PUBLIC_KEY_BYTES<P>> packed_pk;
    std::array<uint8_t, FAEST_SIGNATURE_BYTES<P>> signature;
    test_gen_keypair<P>(packed_pk.data(), packed_sk.data());

    const std::string message =
        "This document describes and specifies the FAEST digital signature algorithm.";

    REQUIRE(faest_sign<P>(signature.data(), reinterpret_cast<const uint8_t*>(message.c_str()),
                          message.size(), packed_sk.data(), NULL, 0));
    REQUIRE(faest_verify<P>(signature.data(), reinterpret_cast<const uint8_t*>(message.c_str()),
                            message.size(), packed_pk.data()));
}
