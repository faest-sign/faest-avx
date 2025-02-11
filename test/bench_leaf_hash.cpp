#include <array>

#include "all.inc"
#include "api.hpp"
#include "parameters.hpp"
#include "test.hpp"
#include "util.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <type_traits>

constexpr size_t MAX_TWEAKS = 16;

template <secpar S>
using aes_ctr_stat_bind_leaf_hash = stat_binding_leaf_hash<aes_ctr_prg<S>, MAX_TWEAKS>;

template <secpar S> using aes_ctr_leaf_hash = prg_leaf_hash<aes_ctr_prg<S>>;

#define LEAF_HASHES(S) shake_leaf_hash<S>, aes_ctr_stat_bind_leaf_hash<S>, aes_ctr_leaf_hash<S>

#define ALL_LEAF_HASHES                                                                            \
    LEAF_HASHES(secpar::s128), LEAF_HASHES(secpar::s192), LEAF_HASHES(secpar::s256)

TEMPLATE_TEST_CASE("bench leaf_hash", "[.][bench][leaf_hash]", ALL_LEAF_HASHES)
{
    using LH = TestType;
    constexpr size_t num_keys = 840;

    std::array<typename LH::key_t, num_keys> keys_in;
    std::generate(keys_in.begin(), keys_in.end(), rand<typename LH::key_t>);
    typename LH::iv_t iv{rand<block128>()};
    auto tweak = rand<typename LH::tweak_t>();
    typename LH::tweak_t small_tweak = 0;

    std::array<uint8_t, num_keys * LH::hash_len> hashes_out;
    std::array<typename LH::key_t, num_keys> keys_out_buf;
    std::array<typename LH::key_t*, num_keys> keys_out;
    std::for_each(keys_out.begin(), keys_out.end(),
                  [&](auto& p) { p = keys_out_buf.data() + ((&p - keys_out.data()) ^ 0x42); });
    assert(keys_out[0] == &keys_out_buf[0x42]);
    assert(keys_out[1] == &keys_out_buf[0x43]);
    assert(keys_out[0x42] == &keys_out_buf[0]);

    auto bench = [&]<typename T>(T)
    {
        constexpr auto chunk_size = T::value;
        static_assert(num_keys % chunk_size == 0);
        BENCHMARK(std::format("hash<{}> - {} keys", chunk_size, num_keys))
        {
            for (size_t i = 0; i < num_keys; i += chunk_size)
                LH::template hash<chunk_size>(&keys_in[i], iv, tweak, small_tweak, &keys_out[i],
                                              &hashes_out[i * LH::hash_len]);
        };
    };

    bench(std::integral_constant<size_t, 1>{});
    bench(std::integral_constant<size_t, 2>{});
    bench(std::integral_constant<size_t, 3>{});
    bench(std::integral_constant<size_t, 4>{});
    bench(std::integral_constant<size_t, 5>{});
    bench(std::integral_constant<size_t, 6>{});
    bench(std::integral_constant<size_t, 7>{});
    bench(std::integral_constant<size_t, 8>{});

    std::cout << "{\n"
              << R"(    "secpar": )" << secpar_to_bits(LH::secpar_v) << ",\n"
              << R"(    "hash_size": )" << LH::hash_len << "\n"
              << "}";
}

template <leaf_hash LH>
using bavc_128_s = one_tree_bavc<secpar::s128, 11, 121, prg::aes_ctr, LH, 3, 102>;
template <leaf_hash LH>
using bavc_128_f = one_tree_bavc<secpar::s128, 16, 120, prg::aes_ctr, LH, 3, 110>;
template <leaf_hash LH>
using bavc_192_s = one_tree_bavc<secpar::s192, 16, 180, prg::aes_ctr, LH, 3, 162>;
template <leaf_hash LH>
using bavc_192_f = one_tree_bavc<secpar::s192, 24, 184, prg::aes_ctr, LH, 3, 163>;
template <leaf_hash LH>
using bavc_256_s = one_tree_bavc<secpar::s256, 22, 250, prg::aes_ctr, LH, 3, 245>;
template <leaf_hash LH>
using bavc_256_f = one_tree_bavc<secpar::s256, 32, 248, prg::aes_ctr, LH, 3, 246>;

#define BAVC_BENCH_INSTANCES(LH)                                                                   \
    bavc_128_s<LH>, bavc_128_f<LH>, bavc_192_s<LH>, bavc_192_f<LH>, bavc_256_s<LH>, bavc_256_f<LH>

#define ALL_BAVC_BENCH_INSTANCES                                                                   \
    BAVC_BENCH_INSTANCES(leaf_hash::shake), BAVC_BENCH_INSTANCES(leaf_hash::aes_ctr_stat_bind),    \
        BAVC_BENCH_INSTANCES(leaf_hash::aes_ctr)

TEMPLATE_TEST_CASE("bench commit/grind_open/verify with leaf_hash", "[leaf_hash]",
                   ALL_BAVC_BENCH_INSTANCES)
{
    using bavc_t = TestType;
    constexpr auto S = bavc_t::secpar_v;
    constexpr auto TAU = bavc_t::tau_v;

    block_secpar<S> seed = rand<block_secpar<S>>();
    std::vector<block_secpar<S>> forest(bavc_t::COMMIT_NODES);
    std::vector<block_secpar<S>> leaves_sender(bavc_t::COMMIT_LEAVES);
    std::vector<block_secpar<S>> leaves_receiver(bavc_t::COMMIT_LEAVES);
    std::vector<unsigned char> hashed_leaves_sender(bavc_t::COMMIT_LEAVES *
                                                    bavc_t::leaf_hash_t::hash_len);
    std::vector<unsigned char> hashed_leaves_receiver(bavc_t::COMMIT_LEAVES *
                                                      bavc_t::leaf_hash_t::hash_len);

    std::array<uint8_t, bavc_t::delta_bytes_v> delta = {0};
    std::array<uint8_t, bavc_t::delta_bits_v> delta_bytes = {0};
    block128 iv = rand<block128>();
    std::vector<uint8_t> opening(bavc_t::OPEN_SIZE);
    std::array<uint8_t, 2 * secpar_to_bytes(S)> check_sender;
    std::array<uint8_t, 2 * secpar_to_bytes(S)> check_receiver;

    const auto hash_hashed_leaves =
        [](const unsigned char* __restrict__ hashed_leaves, uint8_t* __restrict__ hash_of_hashes)
    {
        using VC = VECTOR_COMMITMENT_CONSTANTS<TAU, bavc_t::delta_bits_v>;

        hash_state hasher;
        hasher.init(S);
        hash_hashed_leaves_all_same_size<S>(&hasher, hashed_leaves, VC::NUM_MAX_K,
                                            bavc_t::leaf_hash_t::hash_len << VC::MAX_K);
        hash_hashed_leaves_all_same_size<S>(
            &hasher,
            hashed_leaves + bavc_t::leaf_hash_t::hash_len * ((size_t)VC::NUM_MAX_K << VC::MAX_K),
            VC::NUM_MIN_K, bavc_t::leaf_hash_t::hash_len << VC::MIN_K);
        hasher.update_byte(1);
        hasher.finalize(hash_of_hashes, 2 * secpar_to_bytes(S));
    };

    BENCHMARK("commit")
    {
        bavc_t::commit(seed, iv, forest.data(), leaves_sender.data(), hashed_leaves_sender.data());
        hash_hashed_leaves(hashed_leaves_sender.data(), check_sender.data());
    };

    std::array<uint8_t, 128> chal2;
    uint32_t counter;

    BENCHMARK_ADVANCED("grind & open")(Catch::Benchmark::Chronometer meter)
    {
        // Get average grinding time over various challenges, rather than for this specific
        // signature.
        std::generate(chal2.begin(), chal2.end(), rand<uint8_t>);

        meter.measure(
            [&]
            {
                hash_state_x4 grinding_hasher;
                grinding_hasher.init(S);
                grinding_hasher.update_1(chal2.data(), chal2.size());
                bool open_success =
                    grind_and_open<bavc_t>(forest.data(), hashed_leaves_sender.data(), delta.data(),
                                           opening.data(), &grinding_hasher, &counter);
                assert(open_success);
                (void)open_success;
            });
    };

    // Expand Delta to have one byte per bit
    for (size_t i = 0; i < bavc_t::delta_bits_v; ++i)
        delta_bytes[i] = expand_bit_to_byte(delta[i / 8], i % 8);

    BENCHMARK("verify")
    {
        const auto res_verify =
            bavc_t::verify(iv, opening.data(), delta_bytes.data(), leaves_receiver.data(),
                           hashed_leaves_receiver.data());
        (void)res_verify;
        hash_hashed_leaves(hashed_leaves_receiver.data(), check_receiver.data());
    };

    std::cout << "{\n"
              << R"(    "secpar": )" << secpar_to_bits(S) << ",\n"
              << R"(    "tau": )" << TAU << ",\n"
              << R"(    "delta_bits": )" << bavc_t::delta_bits_v << ",\n"
              << R"(    "opening_seeds_threshold": )" << bavc_t::opening_seeds_threshold_v << "\n"
              << "}";
}

using namespace faest;

template <leaf_hash LH>
using faest_128_s_variant = parameter_set<secpar::s128, 11, owf::v2, prg::aes_ctr, prg::aes_ctr, LH,
                                          7, {bavc::one_tree, 102}>;
template <leaf_hash LH>
using faest_128_f_variant = parameter_set<secpar::s128, 16, owf::v2, prg::aes_ctr, prg::aes_ctr, LH,
                                          8, {bavc::one_tree, 110}>;
template <leaf_hash LH>
using faest_192_s_variant = parameter_set<secpar::s192, 16, owf::v2, prg::aes_ctr, prg::aes_ctr, LH,
                                          12, {bavc::one_tree, 162}>;
template <leaf_hash LH>
using faest_192_f_variant = parameter_set<secpar::s192, 24, owf::v2, prg::aes_ctr, prg::aes_ctr, LH,
                                          8, {bavc::one_tree, 163}>;
template <leaf_hash LH>
using faest_256_s_variant = parameter_set<secpar::s256, 22, owf::v2, prg::aes_ctr, prg::aes_ctr, LH,
                                          6, {bavc::one_tree, 245}>;
template <leaf_hash LH>
using faest_256_f_variant = parameter_set<secpar::s256, 32, owf::v2, prg::aes_ctr, prg::aes_ctr, LH,
                                          8, {bavc::one_tree, 246}>;

template <leaf_hash LH>
using faest_em_128_s_variant = parameter_set<secpar::s128, 11, owf::v2_em, prg::aes_ctr,
                                             prg::aes_ctr, LH, 7, {bavc::one_tree, 103}>;
template <leaf_hash LH>
using faest_em_128_f_variant = parameter_set<secpar::s128, 16, owf::v2_em, prg::aes_ctr,
                                             prg::aes_ctr, LH, 8, {bavc::one_tree, 112}>;
template <leaf_hash LH>
using faest_em_192_s_variant = parameter_set<secpar::s192, 16, owf::v2_em, prg::aes_ctr,
                                             prg::aes_ctr, LH, 8, {bavc::one_tree, 162}>;
template <leaf_hash LH>
using faest_em_192_f_variant = parameter_set<secpar::s192, 24, owf::v2_em, prg::aes_ctr,
                                             prg::aes_ctr, LH, 8, {bavc::one_tree, 176}>;
template <leaf_hash LH>
using faest_em_256_s_variant = parameter_set<secpar::s256, 22, owf::v2_em, prg::aes_ctr,
                                             prg::aes_ctr, LH, 6, {bavc::one_tree, 218}>;
template <leaf_hash LH>
using faest_em_256_f_variant = parameter_set<secpar::s256, 32, owf::v2_em, prg::aes_ctr,
                                             prg::aes_ctr, LH, 8, {bavc::one_tree, 234}>;

#define BENCH_INSTANCES(LH)                                                                        \
    faest_128_s_variant<LH>, faest_128_f_variant<LH>, faest_192_s_variant<LH>,                     \
        faest_192_f_variant<LH>, faest_256_s_variant<LH>, faest_256_f_variant<LH>,                 \
        faest_em_128_s_variant<LH>, faest_em_128_f_variant<LH>, faest_em_192_s_variant<LH>,        \
        faest_em_192_f_variant<LH>, faest_em_256_s_variant<LH>, faest_em_256_f_variant<LH>

#define ALL_BENCH_INSTANCES                                                                        \
    BENCH_INSTANCES(leaf_hash::shake), BENCH_INSTANCES(leaf_hash::aes_ctr_stat_bind),              \
        BENCH_INSTANCES(leaf_hash::aes_ctr)

TEMPLATE_TEST_CASE("bench vole_commit with leaf_hash", "[.][bench][leaf_hash]", ALL_BENCH_INSTANCES)
{
    using P = TestType;
    using CP = P::CONSTS;
    constexpr auto S = P::secpar_v;
    constexpr auto tau = P::tau_v;

    const auto seed = rand<block_secpar<S>>();
    const auto iv = rand<block128>();

    block_secpar<S>* forest = reinterpret_cast<block_secpar<S>*>(
        aligned_alloc(alignof(block_secpar<S>), P::bavc_t::COMMIT_NODES * sizeof(block_secpar<S>)));
    unsigned char* hashed_leaves = reinterpret_cast<unsigned char*>(aligned_alloc(
        alignof(block_2secpar<S>), P::bavc_t::COMMIT_LEAVES * P::leaf_hash_t::hash_len));
    vole_block* u = reinterpret_cast<vole_block*>(
        aligned_alloc(alignof(vole_block), CP::VOLE_COL_BLOCKS * sizeof(vole_block)));
    vole_block* v = reinterpret_cast<vole_block*>(aligned_alloc(
        alignof(vole_block), P::secpar_bits * CP::VOLE_COL_BLOCKS * sizeof(vole_block)));
    std::array<uint8_t, (tau - 1) * CP::VOLE_ROWS / 8> commitment;
    std::array<uint8_t, CP::VOLE_COMMIT_CHECK_SIZE> vole_commit_check;
    std::array<uint8_t, P::bavc_t::OPEN_SIZE> bavc_opening;

    BENCHMARK("vole_commit")
    {
        vole_commit<P>(seed, iv, forest, hashed_leaves, u, v, commitment.data(),
                       vole_commit_check.data());
    };

    std::array<uint8_t, CP::QS::CHALLENGE_BYTES> chal2;
    std::array<uint8_t, CP::QS::CHECK_BYTES> qs_check;
    std::array<uint8_t, CP::QS::PROOF_BYTES> qs_proof;
    std::generate(qs_check.begin(), qs_check.end(), rand<uint8_t>);
    std::generate(qs_proof.begin(), qs_proof.end(), rand<uint8_t>);
    std::array<uint8_t, P::secpar_bytes> delta;
    uint32_t counter;

    BENCHMARK_ADVANCED("grind & open")(Catch::Benchmark::Chronometer meter)
    {
        // Get average grinding time over various challenges, rather than for this specific
        // signature.
        std::generate(chal2.begin(), chal2.end(), rand<uint8_t>);

        meter.measure(
            [&]
            {
                hash_state_x4 grinding_hasher;
                grinding_hasher.init(S);
                grinding_hasher.update_1(chal2.data(), chal2.size());
                grinding_hasher.update_1(qs_check.data(), CP::QS::CHECK_BYTES);
                grinding_hasher.update_1(qs_proof.data(), CP::QS::PROOF_BYTES);
                bool open_success = grind_and_open<typename P::bavc_t>(
                    forest, hashed_leaves, delta.data(), bavc_opening.data(), &grinding_hasher,
                    &counter);
                assert(open_success);
                (void)open_success;
            });
    };

    std::array<uint8_t, P::delta_bits_v> delta_bytes;
    expand_bits_to_bytes(delta_bytes.data(), P::delta_bits_v, delta.data());
    std::array<uint8_t, CP::VOLE_COMMIT_CHECK_SIZE> vole_commit_check_verifier;
    vole_block* q = reinterpret_cast<vole_block*>(aligned_alloc(
        alignof(vole_block), P::secpar_bits * CP::VOLE_COL_BLOCKS * sizeof(vole_block)));

    BENCHMARK("vole_reconstruct")
    {
        bool reconstruct_success =
            vole_reconstruct<P>(iv, q, delta_bytes.data(), commitment.data(), bavc_opening.data(),
                                vole_commit_check_verifier.data());
        assert(reconstruct_success);
        (void)reconstruct_success;
    };

    REQUIRE(vole_commit_check_verifier == vole_commit_check);
}

TEMPLATE_TEST_CASE("bench sign with leaf_hash", "[.][bench][leaf_hash]", ALL_BENCH_INSTANCES)
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
}
