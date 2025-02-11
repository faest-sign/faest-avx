#include <array>

#include "all.inc"
#include "api.hpp"
#include "test.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

namespace v1_5
{
constexpr auto owf_v1_5 = owf::aes_ecb_with_zero_sboxes;
constexpr auto owf_v1_5_em = owf::aes_em_with_zero_sboxes;

using faest_128_s = parameter_set<secpar::s128, 11, owf_v1_5, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::shake, 7, {bavc::one_tree, 102}>;
using faest_128_f = parameter_set<secpar::s128, 16, owf_v1_5, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::shake, 8, {bavc::one_tree, 110}>;
using faest_192_s = parameter_set<secpar::s192, 16, owf_v1_5, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::shake, 12, {bavc::one_tree, 162}>;
using faest_192_f = parameter_set<secpar::s192, 24, owf_v1_5, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::shake, 8, {bavc::one_tree, 163}>;
using faest_256_s = parameter_set<secpar::s256, 22, owf_v1_5, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::shake, 6, {bavc::one_tree, 245}>;
using faest_256_f = parameter_set<secpar::s256, 32, owf_v1_5, prg::aes_ctr, prg::aes_ctr,
                                  leaf_hash::shake, 8, {bavc::one_tree, 246}>;

using faest_em_128_s = parameter_set<secpar::s128, 11, owf_v1_5_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::shake, 7, {bavc::one_tree, 103}>;
using faest_em_128_f = parameter_set<secpar::s128, 16, owf_v1_5_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::shake, 8, {bavc::one_tree, 112}>;
using faest_em_192_s = parameter_set<secpar::s192, 16, owf_v1_5_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::shake, 8, {bavc::one_tree, 162}>;
using faest_em_192_f = parameter_set<secpar::s192, 24, owf_v1_5_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::shake, 8, {bavc::one_tree, 176}>;
using faest_em_256_s = parameter_set<secpar::s256, 22, owf_v1_5_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::shake, 6, {bavc::one_tree, 218}>;
using faest_em_256_f = parameter_set<secpar::s256, 32, owf_v1_5_em, prg::aes_ctr, prg::aes_ctr,
                                     leaf_hash::shake, 8, {bavc::one_tree, 234}>;
} // namespace v1_5

#define ALL_FAEST_VARIANTS                                                                         \
    ALL_FAEST_V2_INSTANCES, v1_5::faest_128_s, v1_5::faest_128_f, v1_5::faest_192_s,               \
        v1_5::faest_192_f, v1_5::faest_256_s, v1_5::faest_256_f, v1_5::faest_em_128_s,             \
        v1_5::faest_em_128_f, v1_5::faest_em_192_s, v1_5::faest_em_192_f, v1_5::faest_em_256_s,    \
        v1_5::faest_em_256_f

TEMPLATE_TEST_CASE("bench variants", "[.][bench][faest-variants]", ALL_FAEST_VARIANTS)
{
    using P = TestType;
    using FP = faest_scheme<P>;

    std::array<unsigned char, FP::CRYPTO_SECRETKEYBYTES> sk;
    std::array<unsigned char, FP::CRYPTO_PUBLICKEYBYTES> pk;

    BENCHMARK("keygen") { return FP::crypto_sign_keypair(pk.data(), sk.data()); };

    const std::string message =
        "This document describes and specifies the FAEST digital signature algorithm.";
    std::vector<unsigned char> signed_message(FP::CRYPTO_BYTES + message.size());
    unsigned long long signed_message_len = 0;

    BENCHMARK("sign")
    {
        return FP::crypto_sign(signed_message.data(), &signed_message_len,
                               reinterpret_cast<const unsigned char*>(message.data()),
                               message.size(), sk.data());
    };

    REQUIRE(signed_message_len == signed_message.size());
    std::vector<unsigned char> opened_message(message.size());
    unsigned long long opened_message_len = 0;

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

    std::cout << "{\n"
              << R"(    "sk_size": )" << FP::CRYPTO_SECRETKEYBYTES << ",\n"
              << R"(    "pk_size": )" << FP::CRYPTO_PUBLICKEYBYTES << ",\n"
              << R"(    "sig_size": )" << FP::CRYPTO_BYTES << ",\n"
              << R"(    "secpar": )" << P::secpar_bits << ",\n"
              << R"(    "tau": )" << P::tau_v << ",\n"
              << R"(    "delta_bits": )" << P::bavc_t::delta_bits_v << ",\n"
              << R"(    "open_threshold": )" << P::bavc_t::opening_seeds_threshold_v << "\n"
              << "}";
}
