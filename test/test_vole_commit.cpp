#include <array>
#include <type_traits>
#include <vector>

#include "test.hpp"
#include "test_vole_commit_tvs.hpp"
#include "test_vole_commit_tvs_v2.hpp"
#include "vector_com.inc"
#include "vole_commit.inc"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("commit/open/verify", "[vole commit]", ALL_FAEST_V1_INSTANCES)
{
    using P = TestType;
    using CP = CONSTANTS<P>;
    using VC = CONSTANTS<P>::VEC_COM;
    constexpr auto S = P::secpar_v;
    constexpr auto TAU = P::tau_v;

    block_secpar<S> seed = rand<block_secpar<S>>();
    block128 iv = rand<block128>();
    std::vector<block_secpar<S>> forest(P::bavc_t::COMMIT_NODES);
    std::vector<block_secpar<S>> leaves_sender(P::bavc_t::COMMIT_LEAVES);
    std::vector<block_secpar<S>> leaves_receiver(P::bavc_t::COMMIT_LEAVES);
    std::vector<unsigned char> hashed_leaves_sender(P::bavc_t::COMMIT_LEAVES * P::leaf_hash_t::hash_len);
    std::vector<unsigned char> hashed_leaves_receiver(P::bavc_t::COMMIT_LEAVES * P::leaf_hash_t::hash_len);

    std::vector<vole_block> u(CP::VOLE_COL_BLOCKS);
    std::vector<vole_block> v(P::secpar_bits * CP::VOLE_COL_BLOCKS);
    std::vector<vole_block> q(P::secpar_bits * CP::VOLE_COL_BLOCKS);
    std::vector<uint8_t> commitment((TAU - 1) * CP::VOLE_ROWS / 8);
    std::vector<uint8_t> opening(P::bavc_t::OPEN_SIZE);
    std::array<uint8_t, 2 * P::secpar_bytes> check_sender;
    std::array<uint8_t, 2 * P::secpar_bytes> check_receiver;

    vole_commit<P>(seed, iv, forest.data(), hashed_leaves_sender.data(), u.data(), v.data(),
                   commitment.data(), check_sender.data());

    const size_t delta = 42 % (1 << VC::MIN_K);

    std::vector<uint8_t> delta_bytes(P::secpar_bits, 0);
    for (size_t i = 0, dst = 0; i < TAU; ++i)
    {
        size_t k = i < VC::NUM_MAX_K ? VC::MAX_K : VC::MIN_K;
        expand_bits_to_bytes(&delta_bytes[dst], k, delta);
        dst += k;
    }

    P::bavc_t::open(forest.data(), hashed_leaves_sender.data(), delta_bytes.data(), opening.data());
    vole_reconstruct<P>(iv, q.data(), delta_bytes.data(), commitment.data(), opening.data(),
                        check_receiver.data());

    REQUIRE(check_receiver == check_sender);

    const auto* u_bytes = reinterpret_cast<const uint8_t*>(u.data());
    const auto* v_bytes = reinterpret_cast<const uint8_t*>(v.data());
    const auto* q_bytes = reinterpret_cast<const uint8_t*>(q.data());

    for (size_t i = 0; i < P::secpar_bits; ++i)
    {
        if (delta_bytes[i])
        {
            for (size_t j = 0; j < CP::VOLE_ROWS / 8; ++j)
            {
                REQUIRE((q_bytes[i * CP::VOLE_COL_BLOCKS * sizeof(vole_block) + j] ^ u_bytes[j]) ==
                        v_bytes[i * CP::VOLE_COL_BLOCKS * sizeof(vole_block) + j]);
            }
        }
        else
        {
            for (size_t j = 0; j < CP::VOLE_ROWS / 8; ++j)
            {
                REQUIRE(q_bytes[i * CP::VOLE_COL_BLOCKS * sizeof(vole_block) + j] ==
                        v_bytes[i * CP::VOLE_COL_BLOCKS * sizeof(vole_block) + j]);
            }
        }
    }
}

TEMPLATE_TEST_CASE("commit test vectors", "[vole commit]", v1::faest_128_s, v1::faest_192_s,
                   v1::faest_256_s)
{
    return; // TODO: Generate new test vectors.

    using P = TestType;
    using CP = CONSTANTS<P>;
    using VC = CONSTANTS<P>::VEC_COM;
    constexpr auto S = P::secpar_v;
    constexpr auto TAU = P::tau_v;

    block_secpar<S> seed;
    block128 iv = block128::set_zero();

    using tv = std::conditional_t<
        std::is_same_v<P, v1::faest_128_s>, tv_128s,
        std::conditional_t<std::is_same_v<P, v1::faest_192_s>, tv_192s,
                           std::conditional_t<std::is_same_v<P, v1::faest_256_s>, tv_256s, void>>>;

    memcpy(&seed, tv::seed.data(), P::secpar_bytes);
    std::vector<uint8_t> expected_commitment(tv::corrections.begin(), tv::corrections.end());
    std::vector<uint8_t> expected_u(tv::u.begin(), tv::u.end());
    std::vector<uint8_t> expected_v(tv::v.begin(), tv::v.end());
    std::vector<uint8_t> expected_q(tv::q.begin(), tv::q.end());
    const auto& expected_hcom = tv::hcom;

    std::vector<block_secpar<S>> forest(P::bavc_t::COMMIT_NODES);
    std::vector<unsigned char> hashed_leaves_sender(P::bavc_t::COMMIT_LEAVES * P::leaf_hash_t::hash_len);

    std::vector<vole_block> u(CP::VOLE_COL_BLOCKS);
    std::vector<vole_block> v(P::secpar_bits * CP::VOLE_COL_BLOCKS);
    std::vector<vole_block> q(P::secpar_bits * CP::VOLE_COL_BLOCKS);
    std::vector<uint8_t> commitment((TAU - 1) * CP::VOLE_ROWS / 8);
    std::vector<uint8_t> opening(P::bavc_t::OPEN_SIZE);
    std::array<uint8_t, 2 * P::secpar_bytes> check_sender;
    std::array<uint8_t, 2 * P::secpar_bytes> check_receiver;

    // commit
    vole_commit<P>(seed, iv, forest.data(), hashed_leaves_sender.data(), u.data(), v.data(),
                   commitment.data(), check_sender.data());

    REQUIRE(CP::VOLE_COL_BLOCKS ==
            (CP::VOLE_ROWS + 8 * sizeof(vole_block) - 1) / sizeof(vole_block) / 8);
    std::vector<uint8_t> u_vec(reinterpret_cast<uint8_t*>(u.data()),
                               reinterpret_cast<uint8_t*>(u.data()) + CP::VOLE_ROWS / 8);
    // std::cerr << "commitment = " << commitment << "\n";
    CHECK(commitment == expected_commitment);
    // std::cerr << "u = " << u_vec << "\n";
    CHECK(u_vec == expected_u);

    std::vector<uint8_t> v_vec(P::secpar_bytes * CP::VOLE_ROWS, 0);
    for (size_t i = 0; i < P::secpar_bits; ++i)
    {
        memcpy(&v_vec[i * CP::VOLE_ROWS / 8], v.data() + i * CP::VOLE_COL_BLOCKS,
               CP::VOLE_ROWS / 8);
    }
    // std::cerr << "v = " << v_vec << "\n";
    CHECK(v_vec == expected_v);
    // std::cerr << "h_com = " << check_sender << "\n";
    CHECK(check_sender == expected_hcom);

    // open
    const size_t delta = 42 % (1 << VC::MIN_K);
    std::vector<uint8_t> delta_bytes(P::secpar_bits, 0);
    for (size_t i = 0, dst = 0; i < TAU; ++i)
    {
        size_t k = i < VC::NUM_MAX_K ? VC::MAX_K : VC::MIN_K;
        expand_bits_to_bytes(&delta_bytes[dst], k, delta);
        dst += k;
    }
    P::bavc_t::open(forest.data(), hashed_leaves_sender.data(), delta_bytes.data(), opening.data());

    // reconstruct
    vole_reconstruct<P>(iv, q.data(), delta_bytes.data(), commitment.data(), opening.data(),
                        check_receiver.data());

    REQUIRE(check_receiver == check_sender);
    std::vector<uint8_t> q_vec(P::secpar_bytes * CP::VOLE_ROWS, 0);
    for (size_t i = 0; i < P::secpar_bits; ++i)
    {
        memcpy(&q_vec[i * CP::VOLE_ROWS / 8], q.data() + i * CP::VOLE_COL_BLOCKS,
               CP::VOLE_ROWS / 8);
    }
    // std::cerr << "q = " << q_vec << "\n";
    CHECK(q_vec == expected_q);
}

TEMPLATE_TEST_CASE("commit test vectors v2", "[vole commit]", ALL_FAEST_V2_INSTANCES)
{
    using P = TestType;
    using CP = CONSTANTS<P>;
    constexpr auto S = P::secpar_v;
    constexpr auto TAU = P::tau_v;
    using TVS = vole_commit_tvs<P>;

    const auto hash_buf = [](const void* buf, size_t n)
    {
        std::array<uint8_t, 64> output;
        hash_state hasher;
        hasher.init(secpar::s256);
        hasher.update(buf, n);
        hasher.finalize(output.data(), output.size());
        return output;
    };

    block_secpar<S> seed;
    memcpy(&seed, TVS::seed.data(), sizeof(seed));
    block128 iv;
    memcpy(&iv, TVS::iv.data(), sizeof(iv));

    std::vector<block_secpar<S>> forest(P::bavc_t::COMMIT_NODES);
    std::vector<unsigned char> hashed_leaves_sender(P::bavc_t::COMMIT_LEAVES *
                                                    P::leaf_hash_t::hash_len);

    std::vector<vole_block> u(CP::VOLE_COL_BLOCKS);
    std::vector<vole_block> v(P::secpar_bits * CP::VOLE_COL_BLOCKS);
    std::vector<vole_block> q(P::secpar_bits * CP::VOLE_COL_BLOCKS);
    std::vector<uint8_t> commitment((TAU - 1) * CP::VOLE_ROWS / 8);
    std::vector<uint8_t> opening(P::bavc_t::OPEN_SIZE);
    std::array<uint8_t, 2 * P::secpar_bytes> check_sender;
    std::array<uint8_t, 2 * P::secpar_bytes> check_receiver;

    // commit
    vole_commit<P>(seed, iv, forest.data(), hashed_leaves_sender.data(), u.data(), v.data(),
                   commitment.data(), check_sender.data());

    static_assert(CP::VOLE_COL_BLOCKS ==
                  (CP::VOLE_ROWS + 8 * sizeof(vole_block) - 1) / sizeof(vole_block) / 8);
    // std::remove_const_t<decltype(TVS::u)> u_bytes;
    // memcpy(u_bytes.data(), u.data(), u_bytes.size());
    // CHECK(u_bytes == TVS::u);
    const auto hashed_u = hash_buf(u.data(), CP::VOLE_ROWS / 8);
    CHECK(hashed_u == TVS::hashed_u);

    // for (size_t i = 0; i < P::tau_v - 1; ++i)
    // {
    //     INFO("i = " << i);
    //     std::array<uint8_t, CP::VOLE_ROWS / 8> c_i_bytes;
    //     std::array<uint8_t, CP::VOLE_ROWS / 8> expected_c_i_bytes;
    //     memcpy(c_i_bytes.data(), commitment.data() + i * CP::VOLE_ROWS / 8, CP::VOLE_ROWS / 8);
    //     memcpy(expected_c_i_bytes.data(), TVS::c.data() + i * CP::VOLE_ROWS / 8, CP::VOLE_ROWS / 8);
    //     CHECK(c_i_bytes == expected_c_i_bytes);
    // }
    const auto hashed_commitment =
        hash_buf(commitment.data(), commitment.size() * sizeof(commitment[0]));
    CHECK(hashed_commitment == TVS::hashed_c);

    hash_state hasher;
    hasher.init(secpar::s256);
    for (size_t i = 0; i < P::secpar_bits; ++i)
    {
        // INFO("i = " << i);
        // std::array<uint8_t, CP::VOLE_ROWS / 8> v_i_bytes;
        // std::array<uint8_t, CP::VOLE_ROWS / 8> expected_v_i_bytes;
        // memcpy(v_i_bytes.data(), v.data() + i * CP::VOLE_COL_BLOCKS, CP::VOLE_ROWS / 8);
        // memcpy(expected_v_i_bytes.data(), TVS::v.data() + i * CP::VOLE_ROWS / 8, CP::VOLE_ROWS / 8);
        // CHECK(v_i_bytes == expected_v_i_bytes);
        hasher.update(v.data() + i * CP::VOLE_COL_BLOCKS, CP::VOLE_ROWS / 8);
    }
    std::array<uint8_t, 64> hashed_v;
    hasher.finalize(hashed_v.data(), hashed_v.size());
    CHECK(hashed_v == TVS::hashed_v);

    CHECK(check_sender == TVS::h);

    // XXX: above pass

    // open
    std::array<uint8_t, P::bavc_t::delta_bits_v> delta_bytes = {0};
    for (size_t i = 0; i < delta_bytes.size(); ++i)
    {
        delta_bytes[i] = ((TVS::chall[i / 8] >> (i % 8)) & 1) ? 0xff : 0x00;
    }
    P::bavc_t::open(forest.data(), hashed_leaves_sender.data(), delta_bytes.data(), opening.data());

    // reconstruct
    vole_reconstruct<P>(iv, q.data(), delta_bytes.data(), commitment.data(), opening.data(),
                        check_receiver.data());

    REQUIRE(check_receiver == check_sender);
    hasher.init(secpar::s256);
    for (size_t i = 0; i < P::secpar_bits; ++i)
    {
        // INFO("i = " << i);
        // std::array<uint8_t, CP::VOLE_ROWS / 8> q_i_bytes;
        // std::array<uint8_t, CP::VOLE_ROWS / 8> expected_q_i_bytes;
        // memcpy(q_i_bytes.data(), q.data() + i * CP::VOLE_COL_BLOCKS, CP::VOLE_ROWS / 8);
        // memcpy(expected_q_i_bytes.data(), TVS::q.data() + i * CP::VOLE_ROWS / 8, CP::VOLE_ROWS / 8);
        // CHECK(q_i_bytes == expected_q_i_bytes);
        hasher.update(q.data() + i * CP::VOLE_COL_BLOCKS, CP::VOLE_ROWS / 8);
    }
    std::array<uint8_t, 64> hashed_q;
    hasher.finalize(hashed_q.data(), hashed_q.size());
    CHECK(hashed_q == TVS::hashed_q);
}
