#include <array>

#include "api.hpp"
#include "faest.inc"
#include "hash.hpp"
#include "small_vole.inc"
#include "test.hpp"
#include "vole_commit.inc"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("bench sign_components", "[.][bench]", ALL_FAEST_INSTANCES)
{
    using P = TestType;
    using CP = P::CONSTS;
    using OC = P::OWF_CONSTS;
    constexpr auto S = P::secpar_v;

    std::array<uint8_t, FAEST_SECRET_KEY_BYTES<P>> sk_packed;
    std::array<uint8_t, FAEST_PUBLIC_KEY_BYTES<P>> pk_packed;
    std::array<uint8_t, FAEST_SIGNATURE_BYTES<P>> signature;
    test_gen_keypair<P>(pk_packed.data(), sk_packed.data());

    const uint8_t* random_seed = nullptr;
    const size_t random_seed_len = 0;
    const std::string message =
        "This document describes and specifies the FAEST digital signature algorithm.";
    const auto* msg = reinterpret_cast<const uint8_t*>(message.c_str());
    const size_t msg_len = message.size();

    secret_key<P> sk;
    BENCHMARK("unpack")
    {
        return faest_unpack_sk_and_get_pubkey<P>(pk_packed.data(), sk_packed.data(), &sk);
    };

    block_2secpar<S> mu;
    hash_state hasher;
    BENCHMARK("hash1: mu <- H_2^0(pk || msg)")
    {
        hasher.init(S);
        hasher.update(pk_packed.data(), FAEST_PUBLIC_KEY_BYTES<P>);
        hasher.update(msg, msg_len);
        hasher.update_byte(8 + 0);
        hasher.finalize(&mu, sizeof(mu));
    };

    block_secpar<S> seed;
    block128 iv_pre;
    std::array<uint8_t, sizeof(seed) + sizeof(iv_pre)> seed_iv_pre;
    BENCHMARK("hash2: (r, iv^pre) <- H_3(sk || mu || rho)")
    {
        hasher.init(S);
        hasher.update(&sk.sk, sizeof(sk.sk));
        hasher.update(&mu, sizeof(mu));
        if (random_seed)
            hasher.update(random_seed, random_seed_len);
        hasher.update_byte(3);
        hasher.finalize(seed_iv_pre.data(), sizeof(seed_iv_pre));
    };

    memcpy(&seed, seed_iv_pre.data(), sizeof(seed));
    memcpy(&iv_pre, &seed_iv_pre[sizeof(seed)], sizeof(iv_pre));

    block128 iv;
    BENCHMARK("hash2.5: iv <- H_4(iv^pre)")
    {
        hasher.init(S);
        hasher.update(&iv_pre, sizeof(iv_pre));
        hasher.update_byte(4);
        hasher.finalize(reinterpret_cast<uint8_t*>(&iv), sizeof(iv));
    };

    block_secpar<S>* forest = reinterpret_cast<block_secpar<S>*>(
        aligned_alloc(alignof(block_secpar<S>), P::bavc_t::COMMIT_NODES * sizeof(block_secpar<S>)));
    unsigned char* hashed_leaves = reinterpret_cast<unsigned char*>(aligned_alloc(
        alignof(block_2secpar<S>), P::bavc_t::COMMIT_LEAVES * P::leaf_hash_t::hash_len));
    vole_block* u = reinterpret_cast<vole_block*>(
        aligned_alloc(alignof(vole_block), CP::VOLE_COL_BLOCKS * sizeof(vole_block)));
    vole_block* v = reinterpret_cast<vole_block*>(aligned_alloc(
        alignof(vole_block), P::secpar_bits * CP::VOLE_COL_BLOCKS * sizeof(vole_block)));
    uint8_t vole_commit_check[CP::VOLE_COMMIT_CHECK_SIZE];

    BENCHMARK("vole_commit")
    {
        vole_commit<P>(seed, iv, forest, hashed_leaves, u, v, signature.data(), vole_commit_check);
    };

    std::array<uint8_t, CP::VOLE_CHECK::CHALLENGE_BYTES> chal1;
    BENCHMARK("hash3: chall_1 <- H_2^1(mu || com || c_1 || ... || c_\\tau-1 || iv)")
    {
        hasher.init(S);
        hasher.update(&mu, sizeof(mu));
        hasher.update(vole_commit_check, CP::VOLE_COMMIT_CHECK_SIZE);
        hasher.update(signature.data(), CP::VOLE_COMMIT_SIZE);
        hasher.update(&iv, sizeof(iv));
        hasher.update_byte(8 + 1);
        hasher.finalize(chal1.data(), sizeof(chal1));
    };

    std::array<uint8_t, CP::QS::CHALLENGE_BYTES> chal2;
    BENCHMARK("hash4: chall_2 <- H_2^2(chall_1 || \\tilde{u} || h_V || d) (1)")
    {
        hasher.init(S);
        hasher.update(chal1.data(), sizeof(chal1));
    };

    uint8_t* vole_check_proof = signature.data() + CP::VOLE_COMMIT_SIZE;
    BENCHMARK("vole_check_sender")
    {
        vole_check_sender<P>(u, v, chal1.data(), vole_check_proof, hasher);
    };

    uint8_t* correction = vole_check_proof + CP::VOLE_CHECK::PROOF_BYTES;
    size_t remainder = (OC::WITNESS_BITS / 8) % (16 * CP::VOLE_BLOCK);
    BENCHMARK("corrections")
    {
        for (size_t i = 0; i < CP::WITNESS_BLOCKS - (remainder != 0); ++i)
        {
            vole_block correction_i = u[i] ^ sk.witness[i];
            memcpy(correction + i * sizeof(vole_block), &correction_i, sizeof(vole_block));
        }
        if (remainder)
        {
            vole_block correction_i =
                u[CP::WITNESS_BLOCKS - 1] ^ sk.witness[CP::WITNESS_BLOCKS - 1];
            memcpy(correction + (CP::WITNESS_BLOCKS - 1) * sizeof(vole_block), &correction_i,
                   remainder);
        }
    };

    BENCHMARK("hash4: chall_2 <- H_2^2(chall_1 || \\tilde{u} || h_V || d) (2)")
    {
        hasher.update(correction, OC::WITNESS_BITS / 8);
        hasher.update_byte(8 + 2);
        hasher.finalize(chal2.data(), sizeof(chal2));
    };

    block_secpar<S>* macs = reinterpret_cast<block_secpar<S>*>(aligned_alloc(
        alignof(block_secpar<S>), CP::QUICKSILVER_ROWS_PADDED * sizeof(block_secpar<S>)));

    memcpy(&u[0], &sk.witness[0], OC::WITNESS_BITS / 8);
    static_assert(CP::QUICKSILVER_ROWS_PADDED % TRANSPOSE_BITS_ROWS == 0, "");
    BENCHMARK("transpose")
    {
        transpose_secpar<S>(v, macs, CP::VOLE_COL_STRIDE, CP::QUICKSILVER_ROWS_PADDED);
    };
    free(v);

    uint8_t* qs_proof = correction + OC::WITNESS_BITS / 8;
    std::array<uint8_t, CP::QS::CHECK_BYTES> qs_check;
    BENCHMARK("quicksilver")
    {
        quicksilver_state<S, false, OC::QS_DEGREE> qs((uint8_t*)&u[0], macs, OC::OWF_NUM_CONSTRAINTS,
                                                      chal2.data());
        owf_constraints(&qs, &sk.pk);
        qs.prove(OC::WITNESS_BITS, qs_proof, qs_check.data());
    };

    free(macs);
    free(u);

    uint8_t* veccom_open_start = qs_proof + CP::QS::PROOF_BYTES;
    uint8_t* delta = veccom_open_start + P::bavc_t::OPEN_SIZE;

    uint8_t* iv_pre_dst = delta + sizeof(block_secpar<S>);
    memcpy(iv_pre_dst, &iv_pre, sizeof(iv_pre));

    if constexpr (!P::use_grinding)
    {
        BENCHMARK("hash5: chall_3 <- H_2^3(chall_2 || \\tilde{a}_0 || \\tilde{a}_1 || \\tilde{a}_2)")
        {
            hasher.init(S);
            hasher.update(chal2.data(), sizeof(chal2));
            hasher.update(qs_check.data(), CP::QS::CHECK_BYTES);
            hasher.update(qs_proof, CP::QS::PROOF_BYTES);
            hasher.update_byte(8 + 3);
            hasher.finalize(delta, sizeof(block_secpar<S>));
        };

        std::array<uint8_t, P::delta_bits_v> delta_bytes;
        expand_bits_to_bytes(delta_bytes.data(), P::delta_bits_v, delta);

        BENCHMARK("vector_open")
        {
            P::bavc_t::open(forest, hashed_leaves, delta_bytes.data(), veccom_open_start);
        };

        REQUIRE(iv_pre_dst + sizeof(iv) == signature.data() + FAEST_SIGNATURE_BYTES<P>);
    }
    else
    {
        uint8_t* grinding_counter_dst = iv_pre_dst + sizeof(iv_pre);
        uint32_t counter;
        BENCHMARK("hash5 grind & open")
        {
            // Get average grinding time over various challenges, rather than for this specific
            // signature.
            std::array<uint8_t, CP::QS::CHALLENGE_BYTES> chal2_fake;
            for (size_t i = 0; i < chal2_fake.size(); ++i)
                chal2_fake[i] = rand<uint8_t>();

            // chall_3 <- H_2^3(chall_2 || \tilde{a}_0 || \tilde{a}_1 || \tilde{a}_2 || ctr)
            // Initialize a 4x hasher and hash the common input prefix.
            hash_state_x4 grinding_hasher;
            grinding_hasher.init(S);
            grinding_hasher.update_1(chal2_fake.data(), chal2_fake.size());
            grinding_hasher.update_1(qs_check.data(), CP::QS::CHECK_BYTES);
            grinding_hasher.update_1(qs_proof, CP::QS::PROOF_BYTES);
            bool open_success = grind_and_open<typename P::bavc_t>(
                forest, hashed_leaves, delta, veccom_open_start, &grinding_hasher, &counter);
            // Opening fails with a negligible probability, so we can assume it succeeds.
            FAEST_ASSERT(open_success);
            (void) open_success;
        };

        // chall_3 <- H_2^3(chall_2 || \tilde{a}_0 || \tilde{a}_1 || \tilde{a}_2 || ctr)
        // Initialize a 4x hasher and hash the common input prefix.
        hash_state_x4 grinding_hasher;
        grinding_hasher.init(S);
        grinding_hasher.update_1(chal2.data(), chal2.size());
        grinding_hasher.update_1(qs_check.data(), CP::QS::CHECK_BYTES);
        grinding_hasher.update_1(qs_proof, CP::QS::PROOF_BYTES);
        bool open_success = grind_and_open<typename P::bavc_t>(
            forest, hashed_leaves, delta, veccom_open_start, &grinding_hasher, &counter);
        // Opening fails with a negligible probability, so we can assume it succeeds.
        FAEST_ASSERT(open_success);
        (void) open_success;

        // Store counter in the signature.
        grinding_counter_dst[0] = counter;
        grinding_counter_dst[1] = counter >> 8;
        grinding_counter_dst[2] = counter >> 16;
        grinding_counter_dst[3] = counter >> 24;

        REQUIRE(grinding_counter_dst + P::grinding_counter_size == signature.data() + FAEST_SIGNATURE_BYTES<P>);
    }

    free(forest);
    free(hashed_leaves);
}

TEMPLATE_TEST_CASE("bench vole_commit", "[.][bench]", ALL_FAEST_INSTANCES)
{
    using P = TestType;
    using CP = P::CONSTS;
    using VC = P::CONSTS::VEC_COM;
    using OC = P::OWF_CONSTS;
    constexpr auto S = P::secpar_v;

    std::array<uint8_t, FAEST_SIGNATURE_BYTES<P>> signature;
    auto seed = rand<block_secpar<S>>();
    auto iv = rand<block128>();

    // Prover
    block_secpar<S>* forest = reinterpret_cast<block_secpar<S>*>(
        aligned_alloc(alignof(block_secpar<S>), P::bavc_t::COMMIT_NODES * sizeof(block_secpar<S>)));
    unsigned char* hashed_leaves = reinterpret_cast<unsigned char*>(aligned_alloc(
        alignof(block_2secpar<S>), P::bavc_t::COMMIT_LEAVES * P::leaf_hash_t::hash_len));
    vole_block* u = reinterpret_cast<vole_block*>(
        aligned_alloc(alignof(vole_block), CP::VOLE_COL_BLOCKS * sizeof(vole_block)));
    vole_block* v = reinterpret_cast<vole_block*>(aligned_alloc(
        alignof(vole_block), P::secpar_bits * CP::VOLE_COL_BLOCKS * sizeof(vole_block)));
    vole_block* q = reinterpret_cast<vole_block*>(aligned_alloc(
        alignof(vole_block), P::secpar_bits * CP::VOLE_COL_BLOCKS * sizeof(vole_block)));
    uint8_t vole_commit_check[CP::VOLE_COMMIT_CHECK_SIZE];
    uint8_t* check = vole_commit_check;

    block_secpar<P::secpar_v>* leaves = reinterpret_cast<block_secpar<P::secpar_v>*>(aligned_alloc(
        alignof(block_secpar<S>), P::bavc_t::COMMIT_LEAVES * sizeof(block_secpar<S>)));
    BENCHMARK("vector_commit") { P::bavc_t::commit(seed, iv, forest, leaves, hashed_leaves); };

    BENCHMARK("hash_hashed_leaves") { hash_hashed_leaves<P>(hashed_leaves, check); };

    auto* v_orig = v;
    BENCHMARK("vole_sender")
    {
        v = v_orig;
        uint8_t* commitment = signature.data();
        vole_block correction[CP::VOLE_COL_BLOCKS];
        block_secpar<S>* leaves_iter = leaves;
        for (size_t i = 0; i < P::tau_v; ++i)
        {
            unsigned int k = i < VC::NUM_MAX_K ? VC::MAX_K : VC::MIN_K;
            auto tweak = ((typename P::vole_prg_t::tweak_t) 1 << 31) + i;
            if (!i)
                vole_sender<P>(k, leaves_iter, iv, tweak, NULL, v, u);
            else
            {
                vole_sender<P>(k, leaves_iter, iv, tweak, u, v, correction);
                memcpy(commitment, correction, CP::VOLE_ROWS / 8);
                commitment += CP::VOLE_ROWS / 8;
            }

            leaves_iter += (size_t)1 << k;
            v += CP::VOLE_COL_BLOCKS * k;
        }

        if constexpr (P::zero_bits_in_delta_v > 0)
        {
            // Clear unused VOLE columns (corresponding to 0 bits of Delta).
            memset(v, 0, CP::VOLE_COL_BLOCKS * P::zero_bits_in_delta_v * sizeof(*v));
        }
    };

    free(leaves);

    std::array<uint8_t, secpar_to_bytes(S)> delta;
    uint8_t* veccom_open_start = signature.data() + OC::WITNESS_BITS / 8;
    if constexpr (!P::use_grinding)
    {
        for (size_t i = 0; i < delta.size(); ++i)
            delta[i] = rand<uint8_t>();
        std::array<uint8_t, P::delta_bits_v> delta_bytes;
        expand_bits_to_bytes(delta_bytes.data(), P::delta_bits_v, delta.data());

        BENCHMARK("vector_open")
        {
            P::bavc_t::open(forest, hashed_leaves, delta_bytes.data(), veccom_open_start);
        };
    }
    else
    {
        std::array<uint8_t, CP::QS::PROOF_BYTES> qs_proof;
        std::array<uint8_t, CP::QS::CHECK_BYTES> qs_check;
        qs_proof.fill(0);
        qs_check.fill(0);

        BENCHMARK("vector_grind_and_open")
        {
            std::array<uint8_t, CP::QS::CHALLENGE_BYTES> chal2;
            for (size_t i = 0; i < chal2.size(); ++i)
                chal2[i] = rand<uint8_t>();

            // Initialize a 4x hasher and hash the common input prefix.
            hash_state_x4 grinding_hasher;
            grinding_hasher.init(S);
            grinding_hasher.update_1(chal2.data(), chal2.size());
            grinding_hasher.update_1(qs_check.data(), qs_check.size());
            grinding_hasher.update_1(qs_proof.data(), qs_proof.size());
            uint32_t counter;
            bool open_success = grind_and_open<typename P::bavc_t>(
                forest, hashed_leaves, delta.data(), veccom_open_start, &grinding_hasher, &counter);
            // Opening fails with a negligible probability, so we can assume it succeeds.
            FAEST_ASSERT(open_success);
            (void) open_success;
        };
    }

    free(forest);
    free(hashed_leaves);

    // Verifier
    leaves = reinterpret_cast<block_secpar<S>*>(aligned_alloc(
        alignof(block_secpar<S>), P::bavc_t::COMMIT_LEAVES * sizeof(block_secpar<S>)));
    hashed_leaves = reinterpret_cast<unsigned char*>(aligned_alloc(
        alignof(block_2secpar<S>), P::bavc_t::COMMIT_LEAVES * P::leaf_hash_t::hash_len));

    std::array<uint8_t, P::delta_bits_v> delta_bytes;
    expand_bits_to_bytes(delta_bytes.data(), P::delta_bits_v, delta.data());
    BENCHMARK("vector_verify")
    {
        REQUIRE(P::bavc_t::verify(iv, veccom_open_start, delta_bytes.data(), leaves, hashed_leaves));
    };

    uint8_t check2[CP::VOLE_COMMIT_CHECK_SIZE];
    BENCHMARK("hash_hashed_leaves")
    {
        hash_hashed_leaves<P>(hashed_leaves, check2);
    };
    REQUIRE(memcmp(vole_commit_check, check2, sizeof(vole_commit_check)) == 0);

    vole_block correction[CP::VOLE_COL_BLOCKS];
    if (CP::VOLE_COL_BLOCKS * sizeof(vole_block) != CP::VOLE_ROWS / 8)
        correction[CP::VOLE_COL_BLOCKS - 1] = vole_block::set_zero();

    auto* q_orig = q;
    BENCHMARK("vole_receiver")
    {
        q = q_orig;
        uint8_t* commitment = signature.data();
        block_secpar<S>* leaves_iter = leaves;
        auto* delta_bytes_ptr = delta_bytes.data();
        for (size_t i = 0; i < P::tau_v; ++i)
        {
            unsigned int k = i < VC::NUM_MAX_K ? VC::MAX_K : VC::MIN_K;
            auto tweak = ((typename P::vole_prg_t::tweak_t) 1 << 31) + i;
            if (!i)
                vole_receiver<P>(k, leaves_iter, iv, tweak, NULL, q, delta_bytes_ptr);
            else
            {
                memcpy(correction, commitment, CP::VOLE_ROWS / 8);
                commitment += CP::VOLE_ROWS / 8;
                vole_receiver<P>(k, leaves_iter, iv, tweak, correction, q, delta_bytes_ptr);
            }

            leaves_iter += (size_t)1 << k;
            q += CP::VOLE_COL_BLOCKS * k;
            delta_bytes_ptr += k;
        }

        if constexpr (P::zero_bits_in_delta_v > 0)
        {
            // Clear unused VOLE columns (corresponding to 0 bits of Delta).
            memset(q, 0, CP::VOLE_COL_BLOCKS * P::zero_bits_in_delta_v * sizeof(*q));
        }
    };

    free(hashed_leaves);
    free(leaves);
    free(u);
    free(v_orig);
    free(q_orig);
}

TEMPLATE_TEST_CASE("bench small_vole_sender", "[.][bench]", ALL_FAEST_INSTANCES)
{
    using P = TestType;
    using CP = P::CONSTS;
    using VC = P::CONSTS::VEC_COM;
    constexpr auto S = P::secpar_v;

    constexpr size_t k = VC::MIN_K;

    block_secpar<S> keys[(size_t) 1 << k];
    typename P::vole_prg_t::iv_t iv{};
    typename P::vole_prg_t::expanded_key_t expanded_keys[CP::VOLE_WIDTH];
    typename P::vole_prg_t::block_t raw_prg_output[CP::VOLE_WIDTH * CP::PRG_VOLE_BLOCKS];
    vole_block* u = reinterpret_cast<vole_block*>(
        aligned_alloc(alignof(vole_block), CP::VOLE_COL_BLOCKS * sizeof(vole_block)));
    vole_block* v = reinterpret_cast<vole_block*>(aligned_alloc(
        alignof(vole_block), P::secpar_bits * CP::VOLE_COL_BLOCKS * sizeof(vole_block)));
    memset(&v[0], 0, detail::COL_LEN<P> * k * sizeof(vole_block));
    vole_block correction[CP::VOLE_COL_BLOCKS];

    for (size_t i = 0; i < (size_t) 1 << k; ++i)
    {
        keys[i] = rand<block_secpar<S>>();
    }

    memset(&iv, 0, sizeof(iv));
    typename P::vole_prg_t::tweak_t tweak = 1 << 31;

    vole_block accum[detail::COL_LEN<P>];
    memset(&accum[0], 0, detail::COL_LEN<P> * sizeof(vole_block));
    BENCHMARK("init/gen")
    {
        for (size_t i = 0; i < (size_t) 1 << k; i += CP::VOLE_WIDTH)
        {
            unsigned int output_col = count_trailing_zeros((i + CP::VOLE_WIDTH) | (1 << (k - 1)));

            P::vole_prg_t::template init<CP::VOLE_WIDTH, CP::PRG_VOLE_BLOCKS>(
                &keys[i], expanded_keys, iv, tweak, 0, raw_prg_output);
            detail::process_prg_output<P, false>(0, output_col, accum, v, raw_prg_output);

            for (size_t j = 1; j < detail::COL_LEN<P>; ++j)
            {
                P::vole_prg_t::template gen<CP::VOLE_WIDTH, CP::PRG_VOLE_BLOCKS>(
                    expanded_keys, iv, tweak, j * CP::PRG_VOLE_BLOCKS, raw_prg_output);
                detail::process_prg_output<P, false>(j, output_col, accum, v, raw_prg_output);
            }
        }
    };

    auto* u_or_c_in = u;
    auto* c_out = &correction[0];
    BENCHMARK("output")
    {
        for (size_t j = 0; j < detail::COL_LEN<P>; ++j)
            c_out[j] = u_or_c_in[j] ^ accum[j];
    };

    free(v);
    free(u);
}

#define PRG_INIT_GEN_NUM_KEYS 128

TEMPLATE_TEST_CASE("bench prg_init_gen", "[.][bench]", aes_ctr_prg<secpar::s128>, aes_ctr_prg<secpar::s192>, aes_ctr_prg<secpar::s256>, rijndael_fixed_key_ctr_prg<secpar::s128>, rijndael_fixed_key_ctr_prg<secpar::s256>)
{
    using PRG = TestType;
    constexpr secpar S = PRG::secpar_v;

    constexpr size_t num_keys = PRG_INIT_GEN_NUM_KEYS;
    block_secpar<S> keys[num_keys];
    for (size_t i = 0; i < num_keys; ++i)
        keys[i] = rand<block_secpar<S>>();
    auto iv = rand<typename PRG::iv_t>();
    typename PRG::tweak_t tweak = 1 << 31;
    typename PRG::count_t counter = 0;

    static_assert(num_keys % PRG::PREFERRED_WIDTH == 0);
    typename PRG::expanded_key_t expanded_keys[num_keys];
    typename PRG::block_t output[4 * num_keys];

    BENCHMARK("init<1> - " STRINGIZE(PRG_INIT_GEN_NUM_KEYS) " keys")
    {
        for (size_t i = 0; i < num_keys; i += PRG::PREFERRED_WIDTH)
            PRG::template init<PRG::PREFERRED_WIDTH, 1>(
                &keys[i], &expanded_keys[i], iv, tweak, counter, &output[i]);
    };

    BENCHMARK("init<2> - " STRINGIZE(PRG_INIT_GEN_NUM_KEYS) " keys")
    {
        for (size_t i = 0; i < num_keys; i += PRG::PREFERRED_WIDTH / 2)
            PRG::template init<PRG::PREFERRED_WIDTH / 2, 2>(
                &keys[i], &expanded_keys[i], iv, tweak, counter, &output[2 * i]);
    };

    BENCHMARK("init<3> - " STRINGIZE(PRG_INIT_GEN_NUM_KEYS) " keys")
    {
        for (size_t i = 0; i < num_keys; i += PRG::PREFERRED_WIDTH / 2)
            PRG::template init<PRG::PREFERRED_WIDTH / 2, 3>(
                &keys[i], &expanded_keys[i], iv, tweak, counter, &output[3 * i]);
    };

    BENCHMARK("init<4> - " STRINGIZE(PRG_INIT_GEN_NUM_KEYS) " keys")
    {
        for (size_t i = 0; i < num_keys; i += PRG::PREFERRED_WIDTH / 4)
            PRG::template init<PRG::PREFERRED_WIDTH / 4, 4>(
                &keys[i], &expanded_keys[i], iv, tweak, counter, &output[4 * i]);
    };

    BENCHMARK("init<4> - " STRINGIZE(PRG_INIT_GEN_NUM_KEYS) " keys")
    {
        for (size_t i = 0; i < num_keys; i += PRG::PREFERRED_WIDTH / 2)
            PRG::template init<PRG::PREFERRED_WIDTH / 2, 4>(
                &keys[i], &expanded_keys[i], iv, tweak, counter, &output[4 * i]);
    };

    BENCHMARK("gen<1> - " STRINGIZE(PRG_INIT_GEN_NUM_KEYS) " keys")
    {
        for (size_t i = 0; i < num_keys; i += PRG::PREFERRED_WIDTH)
            PRG::template gen<PRG::PREFERRED_WIDTH, 1>(
                &expanded_keys[i], iv, tweak, counter, &output[i]);
    };

    BENCHMARK("gen<2> - " STRINGIZE(PRG_INIT_GEN_NUM_KEYS) " keys")
    {
        for (size_t i = 0; i < num_keys; i += PRG::PREFERRED_WIDTH / 2)
            PRG::template gen<PRG::PREFERRED_WIDTH / 2, 2>(
                &expanded_keys[i], iv, tweak, counter, &output[2 * i]);
    };

    BENCHMARK("gen<3> - " STRINGIZE(PRG_INIT_GEN_NUM_KEYS) " keys")
    {
        for (size_t i = 0; i < num_keys; i += PRG::PREFERRED_WIDTH / 2)
            PRG::template gen<PRG::PREFERRED_WIDTH / 2, 3>(
                &expanded_keys[i], iv, tweak, counter, &output[3 * i]);
    };

    BENCHMARK("gen<4> - " STRINGIZE(PRG_INIT_GEN_NUM_KEYS) " keys")
    {
        for (size_t i = 0; i < num_keys; i += PRG::PREFERRED_WIDTH / 4)
            PRG::template gen<PRG::PREFERRED_WIDTH / 4, 4>(
                &expanded_keys[i], iv, tweak, counter, &output[4 * i]);
    };
}
