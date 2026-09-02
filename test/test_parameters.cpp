#include "constants.hpp"
#include "faest.hpp"
#include "faest_keys.hpp"
#include "parameters.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace faest;

// Here we check consistency of the computed constants with the FAEST specification.

TEMPLATE_TEST_CASE("v1_faest_128", "[parameters]", v1::faest_128_s, v1::faest_128_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 40);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 40);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 448);
    REQUIRE(OC::OWF_BLOCK_SIZE == 16);
    REQUIRE(OC::OWF_BLOCKS == 1);
    REQUIRE(OC::OWF_ROUNDS == 10);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 200);
    REQUIRE(OC::WITNESS_BITS == 1600);
    REQUIRE(OC::QS_DEGREE == 2);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 32);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 32);
    if constexpr (std::is_same_v<P, v1::faest_128_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 5006);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 6336);
    }
}

TEMPLATE_TEST_CASE("v1_faest_192", "[parameters]", v1::faest_192_s, v1::faest_192_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 32);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 32);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 448);
    REQUIRE(OC::OWF_BLOCK_SIZE == 16);
    REQUIRE(OC::OWF_BLOCKS == 2);
    REQUIRE(OC::OWF_ROUNDS == 12);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 416);
    REQUIRE(OC::WITNESS_BITS == 3264);
    REQUIRE(OC::QS_DEGREE == 2);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 56);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 64);
    if constexpr (std::is_same_v<P, v1::faest_192_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 12744);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 16792);
    }
}

TEMPLATE_TEST_CASE("v1_faest_256", "[parameters]", v1::faest_256_s, v1::faest_256_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 52);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 52);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 672);
    REQUIRE(OC::OWF_BLOCK_SIZE == 16);
    REQUIRE(OC::OWF_BLOCKS == 2);
    REQUIRE(OC::OWF_ROUNDS == 14);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 500);
    REQUIRE(OC::WITNESS_BITS == 4000);
    REQUIRE(OC::QS_DEGREE == 2);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 64);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 64);
    if constexpr (std::is_same_v<P, v1::faest_256_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 22100);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 28400);
    }
}

TEMPLATE_TEST_CASE("v1_faest_em_128", "[parameters]", v1::faest_em_128_s, v1::faest_em_128_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 0);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 0);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 128);
    REQUIRE(OC::OWF_BLOCK_SIZE == 16);
    REQUIRE(OC::OWF_BLOCKS == 1);
    REQUIRE(OC::OWF_ROUNDS == 10);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 160);
    REQUIRE(OC::WITNESS_BITS == 1280);
    REQUIRE(OC::QS_DEGREE == 2);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 32);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 32);
    if constexpr (std::is_same_v<P, v1::faest_em_128_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 4566);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 5696);
    }
}

TEMPLATE_TEST_CASE("v1_faest_em_192", "[parameters]", v1::faest_em_192_s, v1::faest_em_192_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 0);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 0);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 192);
    REQUIRE(OC::OWF_BLOCK_SIZE == 24);
    REQUIRE(OC::OWF_BLOCKS == 1);
    REQUIRE(OC::OWF_ROUNDS == 12);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 288);
    REQUIRE(OC::WITNESS_BITS == 2304);
    REQUIRE(OC::QS_DEGREE == 2);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 48);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 48);
    if constexpr (std::is_same_v<P, v1::faest_em_192_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 10824);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 13912);
    }
}

TEMPLATE_TEST_CASE("v1_faest_em_256", "[parameters]", v1::faest_em_256_s, v1::faest_em_256_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 0);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 0);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 256);
    REQUIRE(OC::OWF_BLOCK_SIZE == 32);
    REQUIRE(OC::OWF_BLOCKS == 1);
    REQUIRE(OC::OWF_ROUNDS == 14);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 448);
    REQUIRE(OC::WITNESS_BITS == 3584);
    REQUIRE(OC::QS_DEGREE == 2);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 64);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 64);
    if constexpr (std::is_same_v<P, v1::faest_em_256_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 20956);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 26736);
    }
}

TEMPLATE_TEST_CASE("v2_faest_128", "[parameters]", v2::faest_128_s, v2::faest_128_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 40);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 80);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 448);
    REQUIRE(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 832);
    REQUIRE(OC::OWF_BLOCK_SIZE == 16);
    REQUIRE(OC::OWF_BLOCKS == 1);
    REQUIRE(OC::OWF_ROUNDS == 10);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 321);
    REQUIRE(OC::WITNESS_BITS == 1280);
    REQUIRE(OC::QS_DEGREE == 3);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 32);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 32);
    if constexpr (std::is_same_v<P, v2::faest_128_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 4506);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 5924);
    }
}

TEMPLATE_TEST_CASE("v2_faest_192", "[parameters]", v2::faest_192_s, v2::faest_192_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 32);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 64);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 448);
    REQUIRE(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 1024);
    REQUIRE(OC::OWF_BLOCK_SIZE == 16);
    REQUIRE(OC::OWF_BLOCKS == 2);
    REQUIRE(OC::OWF_ROUNDS == 12);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 641);
    REQUIRE(OC::WITNESS_BITS == 2496);
    REQUIRE(OC::QS_DEGREE == 3);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 40);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 48);
    if constexpr (std::is_same_v<P, v2::faest_192_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 11260);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 14948);
    }
}

TEMPLATE_TEST_CASE("v2_faest_256", "[parameters]", v2::faest_256_s, v2::faest_256_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 52);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 104);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 672);
    REQUIRE(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 1216);
    REQUIRE(OC::OWF_BLOCK_SIZE == 16);
    REQUIRE(OC::OWF_BLOCKS == 2);
    REQUIRE(OC::OWF_ROUNDS == 14);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 777);
    REQUIRE(OC::WITNESS_BITS == 3104);
    REQUIRE(OC::QS_DEGREE == 3);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 48);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 48);
    if constexpr (std::is_same_v<P, v2::faest_256_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 20696);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 26548);
    }
}

TEMPLATE_TEST_CASE("v2_faest_em_128", "[parameters]", v2::faest_em_128_s, v2::faest_em_128_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 0);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 0);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 128);
    REQUIRE(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 832);
    REQUIRE(OC::OWF_BLOCK_SIZE == 16);
    REQUIRE(OC::OWF_BLOCKS == 1);
    REQUIRE(OC::OWF_ROUNDS == 10);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 241);
    REQUIRE(OC::WITNESS_BITS == 960);
    REQUIRE(OC::QS_DEGREE == 3);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 32);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 32);
    if constexpr (std::is_same_v<P, v2::faest_em_128_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 3906);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 5060);
    }
}

TEMPLATE_TEST_CASE("v2_faest_em_192", "[parameters]", v2::faest_em_192_s, v2::faest_em_192_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 0);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 0);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 192);
    REQUIRE(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 1536);
    REQUIRE(OC::OWF_BLOCK_SIZE == 24);
    REQUIRE(OC::OWF_BLOCKS == 1);
    REQUIRE(OC::OWF_ROUNDS == 12);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 433);
    REQUIRE(OC::WITNESS_BITS == 1728);
    REQUIRE(OC::QS_DEGREE == 3);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 48);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 48);
    if constexpr (std::is_same_v<P, v2::faest_em_192_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 9340);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 12380);
    }
}

TEMPLATE_TEST_CASE("v2_faest_em_256", "[parameters]", v2::faest_em_256_s, v2::faest_em_256_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    REQUIRE(OC::OWF_KEY_SCHEDULE_SBOXES == 0);
    REQUIRE(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 0);
    REQUIRE(OC::OWF_KEY_WITNESS_BITS == 256);
    REQUIRE(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 2432);
    REQUIRE(OC::OWF_BLOCK_SIZE == 32);
    REQUIRE(OC::OWF_BLOCKS == 1);
    REQUIRE(OC::OWF_ROUNDS == 14);
    REQUIRE(OC::OWF_NUM_CONSTRAINTS == 673);
    REQUIRE(OC::WITNESS_BITS == 2688);
    REQUIRE(OC::QS_DEGREE == 3);
    REQUIRE(FAEST_SECRET_KEY_BYTES<P> == 64);
    REQUIRE(FAEST_PUBLIC_KEY_BYTES<P> == 64);
    if constexpr (std::is_same_v<P, v2::faest_em_256_s>)
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 17984);
    }
    else
    {
        REQUIRE(FAEST_SIGNATURE_BYTES<P> == 23476);
    }
}

TEMPLATE_TEST_CASE("v3_faest_128", "[parameters]", v3::faest_128_s, v3::faest_128_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    CHECK(OC::OWF_KEY_SCHEDULE_SBOXES == 40);
    CHECK(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 80);
    CHECK(OC::OWF_KEY_WITNESS_BITS == 448);
    CHECK(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 512);
    CHECK(OC::OWF_BLOCK_SIZE == 16);
    CHECK(OC::OWF_BLOCKS == 1);
    CHECK(OC::OWF_ROUNDS == 10);
    CHECK(OC::OWF_NUM_CONSTRAINTS == 1121);
    CHECK(OC::WITNESS_BITS == 960);
    CHECK(OC::QS_DEGREE == 7);
    CHECK(FAEST_SECRET_KEY_BYTES<P> == 32);
    CHECK(FAEST_PUBLIC_KEY_BYTES<P> == 32);
    if constexpr (std::is_same_v<P, v3::faest_128_s>)
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 4066);
    }
    else
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 5170);
    }
}

TEMPLATE_TEST_CASE("v3_faest_192", "[parameters]", v3::faest_192_s, v3::faest_192_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    CHECK(OC::OWF_KEY_SCHEDULE_SBOXES == 32);
    CHECK(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 64);
    CHECK(OC::OWF_KEY_WITNESS_BITS == 448);
    CHECK(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 640);
    CHECK(OC::OWF_BLOCK_SIZE == 16);
    CHECK(OC::OWF_BLOCKS == 2);
    CHECK(OC::OWF_ROUNDS == 12);
    CHECK(OC::OWF_NUM_CONSTRAINTS == 1985);
    CHECK(OC::WITNESS_BITS == 1728);
    CHECK(OC::QS_DEGREE == 7);
    CHECK(FAEST_SECRET_KEY_BYTES<P> == 40);
    CHECK(FAEST_PUBLIC_KEY_BYTES<P> == 48);
    if constexpr (std::is_same_v<P, v3::faest_192_s>)
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 9410);
    }
    else
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 11738);
    }
}

TEMPLATE_TEST_CASE("v3_faest_256", "[parameters]", v3::faest_256_s, v3::faest_256_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    CHECK(OC::OWF_KEY_SCHEDULE_SBOXES == 52);
    CHECK(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 104);
    CHECK(OC::OWF_KEY_WITNESS_BITS == 672);
    CHECK(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 768);
    CHECK(OC::OWF_BLOCK_SIZE == 16);
    CHECK(OC::OWF_BLOCKS == 2);
    CHECK(OC::OWF_ROUNDS == 14);
    CHECK(OC::OWF_NUM_CONSTRAINTS == 2537);
    CHECK(OC::WITNESS_BITS == 2208);
    CHECK(OC::QS_DEGREE == 7);
    CHECK(FAEST_SECRET_KEY_BYTES<P> == 48);
    CHECK(FAEST_PUBLIC_KEY_BYTES<P> == 48);
    if constexpr (std::is_same_v<P, v3::faest_256_s>)
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 16626);
    }
    else
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 20856);
    }
}

TEMPLATE_TEST_CASE("v3_faest_em_128", "[parameters]", v3::faest_em_128_s, v3::faest_em_128_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    CHECK(OC::OWF_KEY_SCHEDULE_SBOXES == 0);
    CHECK(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 0);
    CHECK(OC::OWF_KEY_WITNESS_BITS == 128);
    CHECK(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 512);
    CHECK(OC::OWF_BLOCK_SIZE == 16);
    CHECK(OC::OWF_BLOCKS == 1);
    CHECK(OC::OWF_ROUNDS == 10);
    CHECK(OC::OWF_NUM_CONSTRAINTS == 721);
    CHECK(OC::WITNESS_BITS == 640);
    CHECK(OC::QS_DEGREE == 7);
    CHECK(FAEST_SECRET_KEY_BYTES<P> == 32);
    CHECK(FAEST_PUBLIC_KEY_BYTES<P> == 32);
    if constexpr (std::is_same_v<P, v3::faest_em_128_s>)
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 3466);
    }
    else
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 4170);
    }
}

TEMPLATE_TEST_CASE("v3_faest_em_192", "[parameters]", v3::faest_em_192_s, v3::faest_em_192_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    CHECK(OC::OWF_KEY_SCHEDULE_SBOXES == 0);
    CHECK(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 0);
    CHECK(OC::OWF_KEY_WITNESS_BITS == 192);
    CHECK(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 960);
    CHECK(OC::OWF_BLOCK_SIZE == 24);
    CHECK(OC::OWF_BLOCKS == 1);
    CHECK(OC::OWF_ROUNDS == 12);
    CHECK(OC::OWF_NUM_CONSTRAINTS == 1297);
    CHECK(OC::WITNESS_BITS == 1152);
    CHECK(OC::QS_DEGREE == 7);
    CHECK(FAEST_SECRET_KEY_BYTES<P> == 48);
    CHECK(FAEST_PUBLIC_KEY_BYTES<P> == 48);
    if constexpr (std::is_same_v<P, v3::faest_em_192_s>)
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 7874);
    }
    else
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 9818);
    }
}

TEMPLATE_TEST_CASE("v3_faest_em_256", "[parameters]", v3::faest_em_256_s, v3::faest_em_256_f)
{
    using P = TestType;
    using OC = P::OWF_CONSTS;
    CHECK(OC::OWF_KEY_SCHEDULE_SBOXES == 0);
    CHECK(OC::OWF_KEY_SCHEDULE_CONSTRAINTS == 0);
    CHECK(OC::OWF_KEY_WITNESS_BITS == 256);
    CHECK(OC::OWF_ENC_WITNESS_BITS_PER_BLOCK == 1536);
    CHECK(OC::OWF_BLOCK_SIZE == 32);
    CHECK(OC::OWF_BLOCKS == 1);
    CHECK(OC::OWF_ROUNDS == 14);
    CHECK(OC::OWF_NUM_CONSTRAINTS == 2017);
    CHECK(OC::WITNESS_BITS == 1792);
    CHECK(OC::QS_DEGREE == 7);
    CHECK(FAEST_SECRET_KEY_BYTES<P> == 64);
    CHECK(FAEST_PUBLIC_KEY_BYTES<P> == 64);
    if constexpr (std::is_same_v<P, v3::faest_em_256_s>)
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 14554);
    }
    else
    {
        CHECK(FAEST_SIGNATURE_BYTES<P> == 18084);
    }
}
