#include <array>

#include "faest_keys.hpp"
#include "faest_sig.hpp"
#include "test.hpp"
#include "test_faest_tvs.hpp"
#include "test_witness.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "debug.hpp"
#include <print>

TEMPLATE_TEST_CASE("unpack sk", "[faest]", ALL_FAEST_INSTANCES)
{
    using P = TestType;
    using TVS = faest_tvs<P>;
    std::array<uint8_t, FAEST_PUBLIC_KEY_BYTES<P>> packed_pk;
    std::array<uint8_t, (P::OWF_CONSTS::WITNESS_BITS + 7) / 8> witness;

    static_assert(TVS::sk.size() == FAEST_SECRET_KEY_BYTES<P>);
    static_assert(TVS::pk.size() == FAEST_PUBLIC_KEY_BYTES<P>);

    const auto sk = secret_key<P>::unpack(TVS::sk.data());
    REQUIRE(sk);
    sk->pk.pack(packed_pk.data());
    CHECK(packed_pk == TVS::pk);

    memcpy(witness.data(), sk->witness.data(), witness.size());
    CHECK(witness == TVS::witness);
}

TEMPLATE_TEST_CASE("compute pk", "[faest]", ALL_FAEST_INSTANCES)
{
    using P = TestType;
    using TVS = faest_tvs<P>;
    std::array<uint8_t, FAEST_PUBLIC_KEY_BYTES<P>> packed_pk;

    REQUIRE(faest_pubkey<P>(packed_pk.data(), TVS::sk.data()));
    CHECK(packed_pk == TVS::pk);
}

TEMPLATE_TEST_CASE("keygen/sign/verify", "[faest]", ALL_FAEST_INSTANCES)
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
    CHECK(faest_signature<P>(signature.data()).check_format());
    REQUIRE(faest_verify<P>(signature.data(), reinterpret_cast<const uint8_t*>(message.c_str()),
                            message.size(), packed_pk.data()));
}

TEMPLATE_TEST_CASE("sign test vector", "[faest]", ALL_FAEST_INSTANCES)
{
    using P = TestType;
    using TVS = faest_tvs<P>;
    std::array<uint8_t, FAEST_SIGNATURE_BYTES<P>> signature;

    static_assert(TVS::signature.size() == FAEST_SIGNATURE_BYTES<P>);

    REQUIRE(faest_sign<P>(signature.data(), TVS::message.data(), TVS::message.size(),
                          TVS::sk.data(), TVS::random_seed.data(), TVS::random_seed.size()));
    CHECK(signature == TVS::signature);
    CHECK(faest_signature<P>(signature.data()).check_format());

    REQUIRE(faest_verify<P>(signature.data(), TVS::message.data(), TVS::message.size(),
                            TVS::pk.data()));
}
