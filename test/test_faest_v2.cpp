#include <array>

#include "all.inc"
#include "parameters.hpp"
#include "test.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("keygen/sign/verify", "[faest-v2]", ALL_FAEST_V2_INSTANCES)
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
