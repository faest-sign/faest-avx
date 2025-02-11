#include <array>

#include "faest.inc"
#include "faest_keys.hpp"
#include "test.hpp"
#include "test_faest_tvs.hpp"
#include "test_faest_v2_tvs.hpp"
#include "test_witness.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("unpack sk", "[faest]", ALL_FAEST_V1_INSTANCES)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    constexpr auto S = P::secpar_v;
    constexpr auto O = P::owf_v;

    const auto [key, input, output, witness] = []
    {
        if constexpr (O == owf::aes_ecb && S == secpar::s128)
        {
            const auto* key = AES_ECB_128_KEY.data();
            const auto* input = AES_ECB_128_INPUT.data();
            const auto* output = AES_ECB_128_OUTPUT.data();
            const auto* witness = AES_ECB_128_EXTENDED_WITNESS.data();
            REQUIRE(AES_ECB_128_EXTENDED_WITNESS.size() == OC::WITNESS_BITS / 8);
            static_assert(OC::OWF_BLOCK_SIZE == 16);
            static_assert(OC::OWF_BLOCKS == 1);
            return std::make_tuple(key, input, output, witness);
        }
        else if constexpr (O == owf::aes_ecb && S == secpar::s192)
        {
            const auto* key = AES_ECB_192_KEY.data();
            const auto* input = AES_ECB_192_INPUT.data();
            const auto* output = AES_ECB_192_OUTPUT.data();
            const auto* witness = AES_ECB_192_EXTENDED_WITNESS.data();
            REQUIRE(AES_ECB_192_EXTENDED_WITNESS.size() == OC::WITNESS_BITS / 8);
            static_assert(OC::OWF_BLOCK_SIZE == 16);
            static_assert(OC::OWF_BLOCKS == 2);
            return std::make_tuple(key, input, output, witness);
        }
        else if constexpr (O == owf::aes_ecb && S == secpar::s256)
        {
            const auto* key = AES_ECB_256_KEY.data();
            const auto* input = AES_ECB_256_INPUT.data();
            const auto* output = AES_ECB_256_OUTPUT.data();
            const auto* witness = AES_ECB_256_EXTENDED_WITNESS.data();
            REQUIRE(AES_ECB_256_EXTENDED_WITNESS.size() == OC::WITNESS_BITS / 8);
            static_assert(OC::OWF_BLOCK_SIZE == 16);
            static_assert(OC::OWF_BLOCKS == 2);
            return std::make_tuple(key, input, output, witness);
        }
        else if constexpr (O == owf::aes_em && S == secpar::s128)
        {
            const auto* key = RIJNDAEL_EM_128_KEY.data();
            const auto* input = RIJNDAEL_EM_128_INPUT.data();
            const auto* output = RIJNDAEL_EM_128_OUTPUT.data();
            const auto* witness = RIJNDAEL_EM_128_EXTENDED_WITNESS.data();
            REQUIRE(RIJNDAEL_EM_128_EXTENDED_WITNESS.size() == OC::WITNESS_BITS / 8);
            static_assert(OC::OWF_BLOCK_SIZE == 16);
            static_assert(OC::OWF_BLOCKS == 1);
            return std::make_tuple(key, input, output, witness);
        }
        else if constexpr (O == owf::aes_em && S == secpar::s192)
        {
            const auto* key = RIJNDAEL_EM_192_KEY.data();
            const auto* input = RIJNDAEL_EM_192_INPUT.data();
            const auto* output = RIJNDAEL_EM_192_OUTPUT.data();
            const auto* witness = RIJNDAEL_EM_192_EXTENDED_WITNESS.data();
            REQUIRE(RIJNDAEL_EM_192_EXTENDED_WITNESS.size() == OC::WITNESS_BITS / 8);
            static_assert(OC::OWF_BLOCK_SIZE == 24);
            static_assert(OC::OWF_BLOCKS == 1);
            return std::make_tuple(key, input, output, witness);
        }
        else if constexpr (O == owf::aes_em && S == secpar::s256)
        {
            const auto* key = RIJNDAEL_EM_256_KEY.data();
            const auto* input = RIJNDAEL_EM_256_INPUT.data();
            const auto* output = RIJNDAEL_EM_256_OUTPUT.data();
            const auto* witness = RIJNDAEL_EM_256_EXTENDED_WITNESS.data();
            REQUIRE(RIJNDAEL_EM_256_EXTENDED_WITNESS.size() == OC::WITNESS_BITS / 8);
            static_assert(OC::OWF_BLOCK_SIZE == 32);
            static_assert(OC::OWF_BLOCKS == 1);
            return std::make_tuple(key, input, output, witness);
        }
        else
        {
            static_assert(false);
        }
    }();

    std::array<uint8_t, OC::OWF_BLOCKS * OC::OWF_BLOCK_SIZE + P::secpar_bytes> packed_sk;
    memcpy(packed_sk.data(), input, OC::OWF_BLOCKS * OC::OWF_BLOCK_SIZE);
    memcpy(packed_sk.data() + OC::OWF_BLOCKS * OC::OWF_BLOCK_SIZE, key, P::secpar_bytes);

    secret_key<P> sk;
    REQUIRE(faest_unpack_secret_key(&sk, packed_sk.data()));

    const auto computed_output = std::vector(reinterpret_cast<uint8_t*>(sk.pk.owf_output),
                                             reinterpret_cast<uint8_t*>(sk.pk.owf_output) +
                                                 OC::OWF_BLOCKS * OC::OWF_BLOCK_SIZE);
    const auto expected_output = std::vector(output, output + OC::OWF_BLOCKS * OC::OWF_BLOCK_SIZE);
    const auto computed_witness =
        std::vector(reinterpret_cast<uint8_t*>(sk.witness),
                    reinterpret_cast<uint8_t*>(sk.witness) + OC::WITNESS_BITS / 8);
    const auto expected_witness = std::vector(witness, witness + OC::WITNESS_BITS / 8);

    CHECK(computed_output == expected_output);
    CHECK(computed_witness == expected_witness);
}

TEMPLATE_TEST_CASE("unpack sk v2", "[faest]", ALL_FAEST_V2_INSTANCES)
{
    using P = TestType;
    using TVS = faest_tvs<P>;
    std::array<uint8_t, FAEST_PUBLIC_KEY_BYTES<P>> packed_pk;
    std::array<uint8_t, (P::OWF_CONSTS::WITNESS_BITS + 7) / 8> witness;

    static_assert(TVS::sk.size() == FAEST_SECRET_KEY_BYTES<P>);
    static_assert(TVS::pk.size() == FAEST_PUBLIC_KEY_BYTES<P>);

    secret_key<P> sk;
    REQUIRE(faest_unpack_secret_key(&sk, TVS::sk.data()));
    faest_pack_public_key(packed_pk.data(), &sk.pk);
    CHECK(packed_pk == TVS::pk);

    memcpy(witness.data(), &sk.witness, witness.size());
    CHECK(witness == TVS::witness);
}

TEMPLATE_TEST_CASE("compute pk v2", "[faest]", ALL_FAEST_V2_INSTANCES)
{
    using P = TestType;
    using TVS = faest_tvs<P>;
    std::array<uint8_t, FAEST_PUBLIC_KEY_BYTES<P>> packed_pk;

    REQUIRE(faest_pubkey<P>(packed_pk.data(), TVS::sk.data()));
    CHECK(packed_pk == TVS::pk);
}

TEMPLATE_TEST_CASE("keygen/sign/verify v1", "[faest]", ALL_FAEST_V1_INSTANCES)
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

TEMPLATE_TEST_CASE("keygen/sign/verify", "[faest]", ALL_FAEST_V2_INSTANCES)
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

TEMPLATE_TEST_CASE("sign test vector", "[faest]", ALL_FAEST_V2_INSTANCES)
{
    using P = TestType;
    using TVS = faest_tvs<P>;
    std::array<uint8_t, FAEST_SIGNATURE_BYTES<P>> signature;

    static_assert(TVS::signature.size() == FAEST_SIGNATURE_BYTES<P>);

    REQUIRE(faest_sign<P>(signature.data(), TVS::message.data(), TVS::message.size(),
                          TVS::sk.data(), TVS::random_seed.data(), TVS::random_seed.size()));
    CHECK(signature == TVS::signature);

    REQUIRE(faest_verify<P>(signature.data(), TVS::message.data(), TVS::message.size(),
                            TVS::pk.data()));
}
