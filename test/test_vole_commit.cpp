#include <array>
#include <vector>

#include "test.hpp"

extern "C" {

#define restrict __restrict__
#include "vole_commit.h"

}

#include "catch_amalgamated.hpp"



TEST_CASE( "commit/open/verify", "[vole commit]" ) {
    block_secpar seed = rand<block_secpar>();
    block128 iv = rand<block128>();
    std::vector<block_secpar> forest(VECTOR_COMMIT_NODES);
    std::vector<block_secpar> leaves_sender(VECTOR_COMMIT_LEAVES);
    std::vector<block_secpar> leaves_receiver(VECTOR_COMMIT_LEAVES);
    std::vector<block_2secpar> hashed_leaves_sender(VECTOR_COMMIT_LEAVES);
    std::vector<block_2secpar> hashed_leaves_receiver(VECTOR_COMMIT_LEAVES);

    std::vector<vole_block> u(VOLE_COL_BLOCKS);
    std::vector<vole_block> v(SECURITY_PARAM * VOLE_COL_BLOCKS);
    std::vector<vole_block> q(SECURITY_PARAM * VOLE_COL_BLOCKS);
    std::vector<uint8_t> commitment((BITS_PER_WITNESS - 1) * VOLE_ROWS / 8);
    std::vector<uint8_t> opening(VECTOR_OPEN_SIZE);
    std::array<uint8_t, 2 * SECURITY_PARAM / 8> check_sender;
    std::array<uint8_t, 2 * SECURITY_PARAM / 8> check_receiver;

    vole_commit(seed, iv, forest.data(), hashed_leaves_sender.data(), u.data(), v.data(), commitment.data(), check_sender.data());

    const size_t delta = 42 % (1 << VOLE_MIN_K);

    std::vector<uint8_t> delta_bytes(SECURITY_PARAM, 0);
    for (size_t i = 0, dst = 0; i < BITS_PER_WITNESS; ++i)
    {
        size_t k = i < VOLES_MAX_K ? VOLE_MAX_K : VOLE_MIN_K;
        expand_bits_to_bytes(&delta_bytes[dst], k, delta);
        dst += k;
    }


    vector_open(forest.data(), hashed_leaves_sender.data(), delta_bytes.data(), opening.data());
    vole_reconstruct(iv, q.data(), delta_bytes.data(), commitment.data(), opening.data(), check_receiver.data());

    REQUIRE( check_receiver == check_sender );

    const auto* u_bytes = reinterpret_cast<const uint8_t*>(u.data());
    const auto* v_bytes = reinterpret_cast<const uint8_t*>(v.data());
    const auto* q_bytes = reinterpret_cast<const uint8_t*>(q.data());

    for (size_t i = 0; i < SECURITY_PARAM; ++i) {
        if (delta_bytes[i]) {
            for (size_t j = 0; j < VOLE_ROWS / 8; ++j) {
                REQUIRE( (q_bytes[i * VOLE_COL_BLOCKS * sizeof(vole_block) + j] ^ u_bytes[j])
                        == v_bytes[i * VOLE_COL_BLOCKS * sizeof(vole_block) + j] );
            }
        } else {
            for (size_t j = 0; j < VOLE_ROWS / 8; ++j) {
                REQUIRE( q_bytes[i * VOLE_COL_BLOCKS * sizeof(vole_block) + j]
                        == v_bytes[i * VOLE_COL_BLOCKS * sizeof(vole_block) + j] );
            }
        }
    }
}


#if defined(OWF_AES_CTR) && defined(TREE_PRG_AES_CTR) && defined(LEAF_PRG_SHAKE) && (((SECURITY_PARAM == 128) && (BITS_PER_WITNESS == 11)) || ((SECURITY_PARAM == 192) && (BITS_PER_WITNESS == 16)) || ((SECURITY_PARAM == 256) && (BITS_PER_WITNESS == 22)))

namespace tv_128s {
    extern const std::array<uint8_t, SECURITY_PARAM / 8> seed;
    extern const std::array<uint8_t, (BITS_PER_WITNESS - 1) * VOLE_ROWS / 8> corrections;
    extern const std::array<uint8_t, VOLE_ROWS / 8> u;
    extern const std::array<uint8_t, SECURITY_PARAM * VOLE_ROWS / 8> v;
    extern const std::array<uint8_t, SECURITY_PARAM * VOLE_ROWS / 8> q;
    extern const std::array<uint8_t, 2 * SECURITY_PARAM / 8> hcom;
}
namespace tv_192s {
    extern const std::array<uint8_t, SECURITY_PARAM / 8> seed;
    extern const std::array<uint8_t, (BITS_PER_WITNESS - 1) * VOLE_ROWS / 8> corrections;
    extern const std::array<uint8_t, VOLE_ROWS / 8> u;
    extern const std::array<uint8_t, SECURITY_PARAM * VOLE_ROWS / 8> v;
    extern const std::array<uint8_t, SECURITY_PARAM * VOLE_ROWS / 8> q;
    extern const std::array<uint8_t, 2 * SECURITY_PARAM / 8> hcom;
}
namespace tv_256s {
    extern const std::array<uint8_t, SECURITY_PARAM / 8> seed;
    extern const std::array<uint8_t, (BITS_PER_WITNESS - 1) * VOLE_ROWS / 8> corrections;
    extern const std::array<uint8_t, VOLE_ROWS / 8> u;
    extern const std::array<uint8_t, SECURITY_PARAM * VOLE_ROWS / 8> v;
    extern const std::array<uint8_t, SECURITY_PARAM * VOLE_ROWS / 8> q;
    extern const std::array<uint8_t, 2 * SECURITY_PARAM / 8> hcom;
}


TEST_CASE( "commit test vectors", "[vole commit]" ) {
    block_secpar seed;
    block128 iv = block128_set_zero();
#if SECURITY_PARAM == 128
    namespace tv = tv_128s;
#elif SECURITY_PARAM == 192
    namespace tv = tv_192s;
#elif SECURITY_PARAM == 256
    namespace tv = tv_256s;
#endif
    memcpy(&seed, tv::seed.data(), SECURITY_PARAM / 8);
    std::vector<uint8_t> expected_commitment(tv::corrections.begin(), tv::corrections.end());
    std::vector<uint8_t> expected_u(tv::u.begin(), tv::u.end());
    std::vector<uint8_t> expected_v(tv::v.begin(), tv::v.end());
    std::vector<uint8_t> expected_q(tv::q.begin(), tv::q.end());
    const auto& expected_hcom = tv::hcom;

    std::vector<block_secpar> forest(VECTOR_COMMIT_NODES);
    std::vector<block_2secpar> hashed_leaves_sender(VECTOR_COMMIT_LEAVES);

    std::vector<vole_block> u(VOLE_COL_BLOCKS);
    std::vector<vole_block> v(SECURITY_PARAM * VOLE_COL_BLOCKS);
    std::vector<vole_block> q(SECURITY_PARAM * VOLE_COL_BLOCKS);
    std::vector<uint8_t> commitment((BITS_PER_WITNESS - 1) * VOLE_ROWS / 8);
    std::vector<uint8_t> opening(VECTOR_OPEN_SIZE);
    std::array<uint8_t, 2 * SECURITY_PARAM / 8> check_sender;
    std::array<uint8_t, 2 * SECURITY_PARAM / 8> check_receiver;

    // commit
    vole_commit(seed, iv, forest.data(), hashed_leaves_sender.data(), u.data(), v.data(), commitment.data(), check_sender.data());

    REQUIRE( VOLE_COL_BLOCKS == (VOLE_ROWS + 8 * sizeof(vole_block) - 1) / sizeof(vole_block) / 8 );
    std::vector<uint8_t> u_vec(reinterpret_cast<uint8_t*>(u.data()),
                               reinterpret_cast<uint8_t*>(u.data()) + VOLE_ROWS / 8);
    // std::cerr << "commitment = " << commitment << "\n";
    CHECK( commitment == expected_commitment );
    // std::cerr << "u = " << u_vec << "\n";
    CHECK( u_vec == expected_u );

    std::vector<uint8_t> v_vec(SECURITY_PARAM * VOLE_ROWS / 8, 0);
    for (size_t i = 0; i < SECURITY_PARAM; ++i) {
        memcpy(&v_vec[i * VOLE_ROWS / 8], v.data() + i * VOLE_COL_BLOCKS, VOLE_ROWS / 8);
    }
    // std::cerr << "v = " << v_vec << "\n";
    CHECK( v_vec == expected_v );
    // std::cerr << "h_com = " << check_sender << "\n";
    CHECK( check_sender == expected_hcom );

    // open
    const size_t delta = 42 % (1 << VOLE_MIN_K);
    std::vector<uint8_t> delta_bytes(SECURITY_PARAM, 0);
    for (size_t i = 0, dst = 0; i < BITS_PER_WITNESS; ++i)
    {
        size_t k = i < VOLES_MAX_K ? VOLE_MAX_K : VOLE_MIN_K;
        expand_bits_to_bytes(&delta_bytes[dst], k, delta);
        dst += k;
    }
    vector_open(forest.data(), hashed_leaves_sender.data(), delta_bytes.data(), opening.data());

    // reconstruct
    vole_reconstruct(iv, q.data(), delta_bytes.data(), commitment.data(), opening.data(), check_receiver.data());

    REQUIRE( check_receiver == check_sender );
    std::vector<uint8_t> q_vec(SECURITY_PARAM * VOLE_ROWS / 8, 0);
    for (size_t i = 0; i < SECURITY_PARAM; ++i) {
        memcpy(&q_vec[i * VOLE_ROWS / 8], q.data() + i * VOLE_COL_BLOCKS, VOLE_ROWS / 8);
    }
    // std::cerr << "q = " << q_vec << "\n";
    CHECK( q_vec == expected_q );
}

#endif
