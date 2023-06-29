#include <array>

#include "test.hpp"
#include "test_faest_tvs.hpp"
#include "test_witness.hpp"

extern "C" {

#include "config.h"
#include "faest_details.h"

}

#include "catch_amalgamated.hpp"


TEST_CASE( "unpack sk", "[faest]" ) {
#if defined(OWF_AES_CTR) && SECURITY_PARAM == 128
    const auto* key = AES_CTR_128_KEY.data();
    const auto* input = AES_CTR_128_INPUT.data();
    const auto* output = AES_CTR_128_OUTPUT.data();
    const auto* witness = AES_CTR_128_EXTENDED_WITNESS.data();
    REQUIRE( AES_CTR_128_EXTENDED_WITNESS.size() == WITNESS_BITS / 8 );
    static_assert( OWF_BLOCK_SIZE == 16 );
    static_assert( OWF_BLOCKS == 1 );
#elif defined(OWF_AES_CTR) && SECURITY_PARAM == 192
    const auto* key = AES_CTR_192_KEY.data();
    const auto* input = AES_CTR_192_INPUT.data();
    const auto* output = AES_CTR_192_OUTPUT.data();
    const auto* witness = AES_CTR_192_EXTENDED_WITNESS.data();
    REQUIRE( AES_CTR_192_EXTENDED_WITNESS.size() == WITNESS_BITS / 8 );
    static_assert( OWF_BLOCK_SIZE == 16 );
    static_assert( OWF_BLOCKS == 2 );
#elif defined(OWF_AES_CTR) && SECURITY_PARAM == 256
    const auto* key = AES_CTR_256_KEY.data();
    const auto* input = AES_CTR_256_INPUT.data();
    const auto* output = AES_CTR_256_OUTPUT.data();
    const auto* witness = AES_CTR_256_EXTENDED_WITNESS.data();
    REQUIRE( AES_CTR_256_EXTENDED_WITNESS.size() == WITNESS_BITS / 8 );
    static_assert( OWF_BLOCK_SIZE == 16 );
    static_assert( OWF_BLOCKS == 2 );
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR) && SECURITY_PARAM == 128
    const auto* key = RIJNDAEL_EM_128_KEY.data();
    const auto* input = RIJNDAEL_EM_128_INPUT.data();
    const auto* output = RIJNDAEL_EM_128_OUTPUT.data();
    const auto* witness = RIJNDAEL_EM_128_EXTENDED_WITNESS.data();
    REQUIRE( RIJNDAEL_EM_128_EXTENDED_WITNESS.size() == WITNESS_BITS / 8 );
    static_assert( OWF_BLOCK_SIZE == 16 );
    static_assert( OWF_BLOCKS == 1 );
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR) && SECURITY_PARAM == 192
    const auto* key = RIJNDAEL_EM_192_KEY.data();
    const auto* input = RIJNDAEL_EM_192_INPUT.data();
    const auto* output = RIJNDAEL_EM_192_OUTPUT.data();
    const auto* witness = RIJNDAEL_EM_192_EXTENDED_WITNESS.data();
    REQUIRE( RIJNDAEL_EM_192_EXTENDED_WITNESS.size() == WITNESS_BITS / 8 );
    static_assert( OWF_BLOCK_SIZE == 24 );
    static_assert( OWF_BLOCKS == 1 );
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR) && SECURITY_PARAM == 256
    const auto* key = RIJNDAEL_EM_256_KEY.data();
    const auto* input = RIJNDAEL_EM_256_INPUT.data();
    const auto* output = RIJNDAEL_EM_256_OUTPUT.data();
    const auto* witness = RIJNDAEL_EM_256_EXTENDED_WITNESS.data();
    REQUIRE( RIJNDAEL_EM_256_EXTENDED_WITNESS.size() == WITNESS_BITS / 8 );
    static_assert( OWF_BLOCK_SIZE == 32 );
    static_assert( OWF_BLOCKS == 1 );
#endif

    std::array<uint8_t, OWF_BLOCKS * OWF_BLOCK_SIZE + SECURITY_PARAM / 8> packed_sk;
    memcpy(packed_sk.data(), input, OWF_BLOCKS * OWF_BLOCK_SIZE);
    memcpy(packed_sk.data() + OWF_BLOCKS * OWF_BLOCK_SIZE, key, SECURITY_PARAM / 8);

    secret_key sk;
    REQUIRE( faest_unpack_secret_key(&sk, packed_sk.data()) );

    const auto computed_output = std::vector(reinterpret_cast<uint8_t*>(sk.pk.owf_output),
                                             reinterpret_cast<uint8_t*>(sk.pk.owf_output) + OWF_BLOCKS * OWF_BLOCK_SIZE);
    const auto expected_output = std::vector(output, output + OWF_BLOCKS * OWF_BLOCK_SIZE);
    const auto computed_witness = std::vector(reinterpret_cast<uint8_t*>(sk.witness),
                                              reinterpret_cast<uint8_t*>(sk.witness) + WITNESS_BITS / 8);
    const auto expected_witness = std::vector(witness, witness + WITNESS_BITS / 8);

    CHECK( computed_output == expected_output );
    CHECK( computed_witness == expected_witness );
}


TEST_CASE( "keygen/sign/verify", "[faest]" ) {
    std::array<uint8_t, FAEST_SECRET_KEY_BYTES> packed_sk;
    std::array<uint8_t, FAEST_PUBLIC_KEY_BYTES> packed_pk;
    std::array<uint8_t, FAEST_SIGNATURE_BYTES> signature;
    test_gen_keypair(packed_pk.data(), packed_sk.data());

    const std::string message = "This document describes and specifies the FAEST digital signature algorithm.";

    REQUIRE( faest_sign(signature.data(), reinterpret_cast<const uint8_t*>(message.c_str()), message.size(), packed_sk.data(), NULL, 0) );
    REQUIRE( faest_verify(signature.data(), reinterpret_cast<const uint8_t*>(message.c_str()), message.size(), packed_pk.data()) );
}


#if defined(TREE_PRG_AES_CTR) && defined(LEAF_PRG_SHAKE) && \
    ((defined(OWF_AES_CTR) && \
    (((SECURITY_PARAM == 128) && (BITS_PER_WITNESS == 11 || BITS_PER_WITNESS == 16)) || \
     ((SECURITY_PARAM == 192) && (BITS_PER_WITNESS == 16 || BITS_PER_WITNESS == 24)) || \
     ((SECURITY_PARAM == 256) && (BITS_PER_WITNESS == 22 || BITS_PER_WITNESS == 32)))) \
     || \
    (defined(OWF_RIJNDAEL_EVEN_MANSOUR) && \
    (((SECURITY_PARAM == 128) && (BITS_PER_WITNESS == 11 || BITS_PER_WITNESS == 16)) || \
     ((SECURITY_PARAM == 192) && (BITS_PER_WITNESS == 16 || BITS_PER_WITNESS == 24)) || \
     ((SECURITY_PARAM == 256) && (BITS_PER_WITNESS == 22 || BITS_PER_WITNESS == 32)))))

TEST_CASE( "test vector", "[faest]" ) {
#if defined(OWF_AES_CTR) && SECURITY_PARAM == 128 && BITS_PER_WITNESS == 11
    namespace tv = faest_tvs::faest_128s_tvs;
#elif defined(OWF_AES_CTR) && SECURITY_PARAM == 128 && BITS_PER_WITNESS == 16
    namespace tv = faest_tvs::faest_128f_tvs;
#elif defined(OWF_AES_CTR) && SECURITY_PARAM == 192 && BITS_PER_WITNESS == 16
    namespace tv = faest_tvs::faest_192s_tvs;
#elif defined(OWF_AES_CTR) && SECURITY_PARAM == 192 && BITS_PER_WITNESS == 24
    namespace tv = faest_tvs::faest_192f_tvs;
#elif defined(OWF_AES_CTR) && SECURITY_PARAM == 256 && BITS_PER_WITNESS == 22
    namespace tv = faest_tvs::faest_256s_tvs;
#elif defined(OWF_AES_CTR) && SECURITY_PARAM == 256 && BITS_PER_WITNESS == 32
    namespace tv = faest_tvs::faest_256f_tvs;
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR) && SECURITY_PARAM == 128 && BITS_PER_WITNESS == 11
    namespace tv = faest_tvs::faest_em_128s_tvs;
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR) && SECURITY_PARAM == 128 && BITS_PER_WITNESS == 16
    namespace tv = faest_tvs::faest_em_128f_tvs;
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR) && SECURITY_PARAM == 192 && BITS_PER_WITNESS == 16
    namespace tv = faest_tvs::faest_em_192s_tvs;
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR) && SECURITY_PARAM == 192 && BITS_PER_WITNESS == 24
    namespace tv = faest_tvs::faest_em_192f_tvs;
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR) && SECURITY_PARAM == 256 && BITS_PER_WITNESS == 22
    namespace tv = faest_tvs::faest_em_256s_tvs;
#elif defined(OWF_RIJNDAEL_EVEN_MANSOUR) && SECURITY_PARAM == 256 && BITS_PER_WITNESS == 32
    namespace tv = faest_tvs::faest_em_256f_tvs;
#endif
    using faest_tvs::message;

    REQUIRE( tv::packed_sk.size() == FAEST_SECRET_KEY_BYTES );
    REQUIRE( tv::packed_pk.size() == FAEST_PUBLIC_KEY_BYTES );
    REQUIRE( tv::signature.size() == FAEST_SIGNATURE_BYTES );

    std::array<uint8_t, FAEST_PUBLIC_KEY_BYTES> packed_pk;
    std::array<uint8_t, FAEST_SIGNATURE_BYTES> signature;

    faest_pubkey(packed_pk.data(), tv::packed_sk.data());
    CHECK( packed_pk == tv::packed_pk );

    REQUIRE( faest_sign(signature.data(), reinterpret_cast<const uint8_t*>(message.c_str()), message.size(), tv::packed_sk.data(), tv::randomness.data(), tv::randomness.size()) );
    CHECK( signature == tv::signature );

    REQUIRE( faest_verify(signature.data(), reinterpret_cast<const uint8_t*>(message.c_str()), message.size(), tv::packed_pk.data()) );
}

#endif
