#include <array>
#include <vector>

#include "test.hpp"

extern "C" {

#define restrict __restrict__
#include "small_vole.h"

}

#include "catch_amalgamated.hpp"



TEST_CASE( "small vole", "[small vole]" ) {
    const size_t k = 10;
    std::vector<block_secpar> sender_keys(1 << k, block_secpar_set_zero());
    std::vector<block_secpar> receiver_keys(1 << k, block_secpar_set_zero());
    std::vector<vole_block> u(VOLE_COL_BLOCKS, vole_block_set_all_8(0));
    std::vector<vole_block> c(VOLE_COL_BLOCKS, vole_block_set_all_8(0));
    std::vector<vole_block> v(k * VOLE_COL_BLOCKS, vole_block_set_all_8(0));
    std::vector<vole_block> q(k * VOLE_COL_BLOCKS, vole_block_set_all_8(0));

    const size_t delta = 42;
    REQUIRE( delta < (1 << k) );

    std::vector<uint8_t> delta_bytes(k, 0);
    expand_bits_to_bytes(delta_bytes.data(), k, delta);

    std::generate(u.begin(), u.end(), rand<vole_block>);

    const auto orig_keys = random_vector<block_secpar>(1 << k);
    for (size_t i = 0; i < (1 << k); ++i) {
        sender_keys[i] = orig_keys[vole_permute_key_index(i)];
        receiver_keys[i] = orig_keys[vole_permute_key_index(i) ^ delta];
    }

    block_secpar fixed_key_iv = rand<block_secpar>();
	prg_vole_fixed_key fixed_key;
    vole_fixed_key_init(&fixed_key, fixed_key_iv);

    block128 iv = rand<block128>();

    vole_sender(k, sender_keys.data(), iv, &fixed_key, u.data(), v.data(), c.data());
    vole_receiver(k, receiver_keys.data(), iv, &fixed_key, c.data(), q.data(), delta_bytes.data());

    const auto u_vec = std::vector(reinterpret_cast<uint8_t*>(u.data()),
                                   reinterpret_cast<uint8_t*>(u.data() + VOLE_COL_BLOCKS));
    REQUIRE( u_vec.size() == VOLE_COL_BLOCKS * sizeof(vole_block) );
    for (size_t i = 0; i < k; ++i) {
        const auto v_vec = std::vector(reinterpret_cast<uint8_t*>(&v[i * VOLE_COL_BLOCKS]),
                                       reinterpret_cast<uint8_t*>(&v[(i + 1) * VOLE_COL_BLOCKS]));
        const auto q_vec = std::vector(reinterpret_cast<uint8_t*>(&q[i * VOLE_COL_BLOCKS]),
                                       reinterpret_cast<uint8_t*>(&q[(i + 1) * VOLE_COL_BLOCKS]));

        REQUIRE( v_vec.size() == VOLE_COL_BLOCKS * sizeof(vole_block) );
        REQUIRE( q_vec.size() == VOLE_COL_BLOCKS * sizeof(vole_block) );
        auto q_xor_u_vec = q_vec;
        REQUIRE( q_xor_u_vec == q_vec );
        REQUIRE( q_xor_u_vec.size() == u_vec.size() );
        REQUIRE( q_vec.size() == u_vec.size() );
        for (size_t j = 0; j < q_vec.size(); ++j) {
            q_xor_u_vec[j] = q_xor_u_vec[j] ^ u_vec[j];
        }
        if ((delta >> i) & 1) {
            CHECK( v_vec == q_xor_u_vec );
        } else {
            CHECK( v_vec == q_vec );
        }
    }
}

TEST_CASE("vole_permute_key_index", "[small vole]") {
    for (size_t i = 0; i < (size_t) 1 << VOLE_MAX_K; ++i)
    {
        REQUIRE(vole_permute_key_index(vole_permute_key_index_inv(i)) == i);
        REQUIRE(vole_permute_key_index_inv(vole_permute_key_index(i)) == i);
    }
}

TEST_CASE("vole_permute_inv_increment", "[small vole]") {
    for (size_t offset = 1; offset <= VOLE_WIDTH; offset <<= 1)
        for (size_t i = 0; i < ((size_t) 1 << VOLE_MAX_K) - offset; ++i)
            REQUIRE((vole_permute_key_index_inv(i) ^ vole_permute_key_index_inv(i + offset)) ==
                    vole_permute_inv_increment(i, offset));
}
