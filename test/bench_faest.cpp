#include <array>
#include <print>

#include "all.inc"
#include "api.hpp"
#include "cycle_count.hpp"
#include "test.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

namespace v1_5
{
constexpr auto owf_v1_5 = owf::aes_ecb_with_zero_sboxes;
constexpr auto owf_v1_5_em = owf::aes_em_with_zero_sboxes;

template <secpar S, std::size_t TAU, bool IS_EM, std::size_t W_GRIND, std::size_t T_OPEN>
using v1_5_parameter_set =
    parameter_set<S, TAU, (IS_EM ? owf_v1_5_em : owf_v1_5), prg::aes_ctr, prg::aes_ctr,
                  leaf_hash::shake, W_GRIND, {bavc::one_tree, T_OPEN}, false, false>;

using faest_128_f = v1_5_parameter_set<secpar::s128, 16, false, 8, 110>;
using faest_128_s = v1_5_parameter_set<secpar::s128, 11, false, 7, 102>;
using faest_192_f = v1_5_parameter_set<secpar::s192, 24, false, 8, 163>;
using faest_192_s = v1_5_parameter_set<secpar::s192, 16, false, 12, 162>;
using faest_256_f = v1_5_parameter_set<secpar::s256, 32, false, 8, 246>;
using faest_256_s = v1_5_parameter_set<secpar::s256, 22, false, 6, 245>;

using faest_em_128_f = v1_5_parameter_set<secpar::s128, 16, true, 8, 112>;
using faest_em_128_s = v1_5_parameter_set<secpar::s128, 11, true, 7, 103>;
using faest_em_192_f = v1_5_parameter_set<secpar::s192, 24, true, 8, 176>;
using faest_em_192_s = v1_5_parameter_set<secpar::s192, 16, true, 8, 162>;
using faest_em_256_f = v1_5_parameter_set<secpar::s256, 32, true, 8, 234>;
using faest_em_256_s = v1_5_parameter_set<secpar::s256, 22, true, 6, 218>;
} // namespace v1_5


#define ALL_FAEST_VARIANTS ALL_FAEST_INSTANCES


TEMPLATE_TEST_CASE("bench variants", "[.][bench][faest-variants]", ALL_FAEST_VARIANTS)
{
    const bool rdpmc_available = cycles_setup();

    using P = TestType;
    using FP = faest_scheme<P>;

    std::array<unsigned char, FP::CRYPTO_SECRETKEYBYTES> sk;
    std::array<unsigned char, FP::CRYPTO_PUBLICKEYBYTES> pk;

    constexpr size_t CYCLE_COUNT_ITER_KEYGEN = 10000;
    constexpr size_t CYCLE_COUNT_ITER_SIGN_VERIFY = 1000;

    double keygen_avg_pmc_cycles = 0;
    double keygen_avg_ref_cycles = 0;
    double sign_avg_pmc_cycles = 0;
    double sign_avg_ref_cycles = 0;
    double verify_avg_pmc_cycles = 0;
    double verify_avg_ref_cycles = 0;

    BENCHMARK("keygen") { return FP::crypto_sign_keypair(pk.data(), sk.data()); };

    {
        const auto cycles_pmc_start = cpucycles_pmc();
        const auto cycles_ref_start = cpucycles_ref();
        for (size_t i = 0; i < CYCLE_COUNT_ITER_KEYGEN; ++i)
            FP::crypto_sign_keypair(pk.data(), sk.data());
        const auto cycles_pmc_end = cpucycles_pmc();
        const auto cycles_ref_end = cpucycles_ref();
        keygen_avg_pmc_cycles =
            rdpmc_available
                ? static_cast<double>(cycles_pmc_end - cycles_pmc_start) / CYCLE_COUNT_ITER_KEYGEN
                : 0;
        keygen_avg_ref_cycles =
            static_cast<double>(cycles_ref_end - cycles_ref_start) / CYCLE_COUNT_ITER_KEYGEN;
    }

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

    {
        const auto cycles_pmc_start = cpucycles_pmc();
        const auto cycles_ref_start = cpucycles_ref();
        for (size_t i = 0; i < CYCLE_COUNT_ITER_SIGN_VERIFY; ++i)
            FP::crypto_sign(signed_message.data(), &signed_message_len,
                            reinterpret_cast<const unsigned char*>(message.data()), message.size(),
                            sk.data());
        const auto cycles_pmc_end = cpucycles_pmc();
        const auto cycles_ref_end = cpucycles_ref();
        sign_avg_pmc_cycles = rdpmc_available
                                  ? static_cast<double>(cycles_pmc_end - cycles_pmc_start) /
                                        CYCLE_COUNT_ITER_SIGN_VERIFY
                                  : 0;
        sign_avg_ref_cycles =
            static_cast<double>(cycles_ref_end - cycles_ref_start) / CYCLE_COUNT_ITER_SIGN_VERIFY;
    }

    REQUIRE(signed_message_len == signed_message.size());
    std::vector<unsigned char> opened_message(message.size());
    unsigned long long opened_message_len = 0;

    BENCHMARK("verify")
    {
        return FP::crypto_sign_open(opened_message.data(), &opened_message_len,
                                    signed_message.data(), signed_message_len, pk.data());
    };

    {
        const auto cycles_pmc_start = cpucycles_pmc();
        const auto cycles_ref_start = cpucycles_ref();
        for (size_t i = 0; i < CYCLE_COUNT_ITER_SIGN_VERIFY; ++i)
            FP::crypto_sign_open(opened_message.data(), &opened_message_len, signed_message.data(),
                                 signed_message_len, pk.data());
        const auto cycles_pmc_end = cpucycles_pmc();
        const auto cycles_ref_end = cpucycles_ref();
        verify_avg_pmc_cycles = rdpmc_available
                                    ? static_cast<double>(cycles_pmc_end - cycles_pmc_start) /
                                          CYCLE_COUNT_ITER_SIGN_VERIFY
                                    : 0;
        verify_avg_ref_cycles =
            static_cast<double>(cycles_ref_end - cycles_ref_start) / CYCLE_COUNT_ITER_SIGN_VERIFY;
    }

    REQUIRE(opened_message_len == opened_message.size());
    REQUIRE(opened_message ==
            std::vector<unsigned char>(reinterpret_cast<const unsigned char*>(message.c_str()),
                                       reinterpret_cast<const unsigned char*>(message.c_str()) +
                                           message.size()));

    std::cout << "{\n"
              << std::format("    \"sk_size\": {:d},\n", FP::CRYPTO_SECRETKEYBYTES)
              << std::format("    \"pk_size\": {:d},\n", FP::CRYPTO_PUBLICKEYBYTES)
              << std::format("    \"sig_size\": {:d},\n", FP::CRYPTO_BYTES)
              << std::format("    \"secpar\": {:d},\n", P::secpar_bits)
              << std::format("    \"tau\": {:d},\n", P::tau_v)
              << std::format("    \"delta_bits\": {:d},\n", P::bavc_t::delta_bits_v)
              << std::format("    \"open_threshold\": {:d},\n",
                             P::bavc_t::opening_seeds_threshold_v)
              << std::format("    \"rdpmc_available\": {:s},\n", rdpmc_available)
              << std::format("    \"keygen_avg_pmc_cycles\": {:f},\n", keygen_avg_pmc_cycles)
              << std::format("    \"keygen_avg_ref_cycles\": {:f},\n", keygen_avg_ref_cycles)
              << std::format("    \"sign_avg_pmc_cycles\": {:f},\n", sign_avg_pmc_cycles)
              << std::format("    \"sign_avg_ref_cycles\": {:f},\n", sign_avg_ref_cycles)
              << std::format("    \"verify_avg_pmc_cycles\": {:f},\n", verify_avg_pmc_cycles)
              << std::format("    \"verify_avg_ref_cycles\": {:f}\n", verify_avg_ref_cycles) << "}";

    cycles_teardown();
}
