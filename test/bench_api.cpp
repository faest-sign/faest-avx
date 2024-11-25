#include <array>

#include "api.hpp"
#include "test.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("bench keygen", "[.][bench]", ALL_FAEST_INSTANCES)
{
    using FP = faest_scheme<TestType>;

    std::array<unsigned char, FP::CRYPTO_SECRETKEYBYTES> sk;
    std::array<unsigned char, FP::CRYPTO_PUBLICKEYBYTES> pk;

    BENCHMARK("keygen") { return FP::crypto_sign_keypair(pk.data(), sk.data()); };
}

TEMPLATE_TEST_CASE("bench sign", "[.][bench]", ALL_FAEST_INSTANCES)
{
    using FP = faest_scheme<TestType>;

    std::array<unsigned char, FP::CRYPTO_SECRETKEYBYTES> sk;
    std::array<unsigned char, FP::CRYPTO_PUBLICKEYBYTES> pk;
    FP::crypto_sign_keypair(pk.data(), sk.data());
    const std::string message =
        "This document describes and specifies the FAEST digital signature algorithm.";
    std::vector<unsigned char> signed_message(FP::CRYPTO_BYTES + message.size());
    unsigned long long signed_message_len;

    BENCHMARK("sign")
    {
        return FP::crypto_sign(signed_message.data(), &signed_message_len,
                               reinterpret_cast<const unsigned char*>(message.data()),
                               message.size(), sk.data());
    };

    REQUIRE(signed_message_len == signed_message.size());
}

TEMPLATE_TEST_CASE("bench verify", "[.][bench]", ALL_FAEST_INSTANCES)
{
    using FP = faest_scheme<TestType>;

    std::array<unsigned char, FP::CRYPTO_SECRETKEYBYTES> sk;
    std::array<unsigned char, FP::CRYPTO_PUBLICKEYBYTES> pk;
    FP::crypto_sign_keypair(pk.data(), sk.data());
    const std::string message =
        "This document describes and specifies the FAEST digital signature algorithm.";
    std::vector<unsigned char> signed_message(FP::CRYPTO_BYTES + message.size());
    unsigned long long signed_message_len;
    FP::crypto_sign(signed_message.data(), &signed_message_len,
                    reinterpret_cast<const unsigned char*>(message.data()), message.size(),
                    sk.data());
    std::vector<unsigned char> opened_message(message.size());
    unsigned long long opened_message_len;

    BENCHMARK("verify")
    {
        return FP::crypto_sign_open(opened_message.data(), &opened_message_len,
                                    signed_message.data(), signed_message_len, pk.data());
    };

    REQUIRE(opened_message_len == opened_message.size());
    REQUIRE(opened_message ==
            std::vector<unsigned char>(reinterpret_cast<const unsigned char*>(message.c_str()),
                                       reinterpret_cast<const unsigned char*>(message.c_str()) +
                                           message.size()));
}
