#include <vector>

#include "prgs.hpp"
#include "small_vole.hpp"
#include "test.hpp"
#include "util.hpp"
#include "vole_key_index_permutation.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("small vole", "[small vole]", ALL_FAEST_V1_INSTANCES)
{
    using P = TestType;
    using CP = CONSTANTS<P>;
    constexpr auto S = P::secpar_v;

    const size_t k = 10;
    std::vector<block_secpar<S>> sender_keys(1 << k, block_secpar<S>::set_zero());
    std::vector<block_secpar<S>> receiver_keys(1 << k, block_secpar<S>::set_zero());
    std::vector<vole_block> u(CP::VOLE_COL_BLOCKS, vole_block::set_all_8(0));
    std::vector<vole_block> c(CP::VOLE_COL_BLOCKS, vole_block::set_all_8(0));
    std::vector<vole_block> v(k * CP::VOLE_COL_BLOCKS, vole_block::set_all_8(0));
    std::vector<vole_block> q(k * CP::VOLE_COL_BLOCKS, vole_block::set_all_8(0));

    const size_t delta = 42;
    REQUIRE(delta < (1 << k));

    std::vector<uint8_t> delta_bytes(k, 0);
    expand_bits_to_bytes(delta_bytes.data(), k, delta);

    std::generate(u.begin(), u.end(), rand<vole_block>);

    const auto orig_keys = random_vector<block_secpar<S>>(1 << k);
    for (size_t i = 0; i < (1 << k); ++i)
    {
        sender_keys[i] = orig_keys[vole_permute_key_index<CP::VOLE_WIDTH_SHIFT>(i)];
        receiver_keys[i] = orig_keys[vole_permute_key_index<CP::VOLE_WIDTH_SHIFT>(i) ^ delta];
    }

    block128 iv = rand<block128>();
    uint32_t tweak = 0xbeef;

    vole_sender<P>(k, sender_keys.data(), iv, tweak, u.data(), v.data(), c.data());
    vole_receiver<P>(k, receiver_keys.data(), iv, tweak, c.data(), q.data(), delta_bytes.data());

    const auto u_vec = std::vector(reinterpret_cast<uint8_t*>(u.data()),
                                   reinterpret_cast<uint8_t*>(u.data() + CP::VOLE_COL_BLOCKS));
    REQUIRE(u_vec.size() == CP::VOLE_COL_BLOCKS * sizeof(vole_block));
    for (size_t i = 0; i < k; ++i)
    {
        const auto v_vec = std::vector(
            reinterpret_cast<uint8_t*>(&v[i * CP::VOLE_COL_BLOCKS]),
            reinterpret_cast<uint8_t*>(&v[i * CP::VOLE_COL_BLOCKS] + CP::VOLE_COL_BLOCKS));
        const auto q_vec = std::vector(
            reinterpret_cast<uint8_t*>(&q[i * CP::VOLE_COL_BLOCKS]),
            reinterpret_cast<uint8_t*>(&q[i * CP::VOLE_COL_BLOCKS] + CP::VOLE_COL_BLOCKS));

        REQUIRE(v_vec.size() == CP::VOLE_COL_BLOCKS * sizeof(vole_block));
        REQUIRE(q_vec.size() == CP::VOLE_COL_BLOCKS * sizeof(vole_block));
        auto q_xor_u_vec = q_vec;
        REQUIRE(q_xor_u_vec == q_vec);
        REQUIRE(q_xor_u_vec.size() == u_vec.size());
        REQUIRE(q_vec.size() == u_vec.size());
        for (size_t j = 0; j < q_vec.size(); ++j)
        {
            q_xor_u_vec[j] = q_xor_u_vec[j] ^ u_vec[j];
        }
        if ((delta >> i) & 1)
        {
            CHECK(v_vec == q_xor_u_vec);
        }
        else
        {
            CHECK(v_vec == q_vec);
        }
    }
}

TEMPLATE_TEST_CASE("vole_permute_key_index", "[small vole]", ALL_FAEST_V1_INSTANCES)
{
    using P = TestType;
    constexpr auto VOLE_WIDTH_SHIFT = P::CONSTS::VOLE_WIDTH_SHIFT;
    constexpr auto MAX_K = P::CONSTS::VEC_COM::MAX_K;
    for (size_t i = 0; i < (size_t)1 << MAX_K; ++i)
    {
        REQUIRE(vole_permute_key_index<VOLE_WIDTH_SHIFT>(
                    vole_permute_key_index_inv<VOLE_WIDTH_SHIFT, MAX_K>(i)) == i);
        REQUIRE(vole_permute_key_index_inv<VOLE_WIDTH_SHIFT, MAX_K>(
                    vole_permute_key_index<VOLE_WIDTH_SHIFT>(i)) == i);
    }
}

TEMPLATE_TEST_CASE("vole_permute_inv_increment", "[small vole]", ALL_FAEST_V1_INSTANCES)
{
    using P = TestType;
    constexpr auto VOLE_WIDTH = P::CONSTS::VOLE_WIDTH;
    constexpr auto VOLE_WIDTH_SHIFT = P::CONSTS::VOLE_WIDTH_SHIFT;
    constexpr auto MAX_K = P::CONSTS::VEC_COM::MAX_K;
    for (size_t offset = 1; offset <= VOLE_WIDTH; offset <<= 1)
        for (size_t i = 0; i < ((size_t)1 << MAX_K) - offset; ++i)
            REQUIRE((vole_permute_key_index_inv<VOLE_WIDTH_SHIFT, MAX_K>(i) ^
                     vole_permute_key_index_inv<VOLE_WIDTH_SHIFT, MAX_K>(i + offset)) ==
                    vole_permute_inv_increment<VOLE_WIDTH_SHIFT, MAX_K>(i, offset));
}
