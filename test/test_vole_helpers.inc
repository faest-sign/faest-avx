#ifndef TEST_VOLE_HELPERS_HPP
#define TEST_VOLE_HELPERS_HPP

#include <catch2/catch_test_macros.hpp>
#include <vector>

#include <block.hpp>
#include <constants.hpp>
#include <parameters.hpp>
#include <polynomials.hpp>
#include <util.hpp>

#include <test.hpp>

using namespace faest;

template <secpar S> block_secpar<S> sample_delta(size_t bits)
{
    auto delta = rand_block<secpar_to_bits(S)>();
    // zero the last bits of delta
    auto* delta_bytes = reinterpret_cast<uint8_t*>(&delta);
    for (size_t i = bits; i < secpar_to_bits(S); ++i)
    {
        delta_bytes[i / 8] &= ~(1 << (i % 8));
    }
    return delta;
}

template <secpar S, size_t DELTA_BITS>
std::tuple<std::vector<vole_block>, std::vector<vole_block>, block_secpar<S>,
           std::vector<vole_block>> inline gen_vole_columns(size_t n)
{
    const auto u = random_vector<vole_block>(n);
    const auto delta = sample_delta<S>(DELTA_BITS);
    auto v = random_vector<vole_block>(secpar_to_bits(S) * n);
    auto q = std::vector<vole_block>(v.size());

    std::vector<uint8_t> delta_bytes(secpar_to_bits(S), 0);
    expand_bits_to_bytes(delta_bytes.data(), DELTA_BITS, reinterpret_cast<const uint8_t*>(&delta));

    // zero the columns corresponding to the zero bits in delta
    std::fill(v.begin() + DELTA_BITS * n, v.end(), vole_block::set_zero());

    // compute q = u * delta + v
    for (size_t i = 0; i < DELTA_BITS; ++i)
    {
        if (delta_bytes[i])
        {
            for (size_t j = 0; j < n; ++j)
                q[i * n + j] = v[i * n + j] ^ u[j];
        }
        else
        {
            for (size_t j = 0; j < n; ++j)
                q[i * n + j] = v[i * n + j];
        }
    }
    return std::make_tuple(u, v, delta, q);
}

template <secpar S>
void inline verify_vole_columns(size_t delta_bits, size_t n_bytes, size_t stride,
                                const vole_block* u, const vole_block* v, block_secpar<S> delta,
                                const vole_block* q)
{
    std::vector<uint8_t> delta_bytes(secpar_to_bits(S), 0);
    expand_bits_to_bytes(delta_bytes.data(), secpar_to_bits(S),
                         reinterpret_cast<const uint8_t*>(&delta));

    const auto* u_bytes = reinterpret_cast<const uint8_t*>(u);
    const auto* v_bytes = reinterpret_cast<const uint8_t*>(v);
    const auto* q_bytes = reinterpret_cast<const uint8_t*>(q);
    for (size_t i = 0; i < delta_bits; ++i)
    {
        INFO("i = " << i);
        if (delta_bytes[i])
        {
            for (size_t j = 0; j < n_bytes; ++j)
            {
                INFO("j = " << j);
                REQUIRE((q_bytes[j] ^ u_bytes[j]) == v_bytes[j]);
            }
        }
        else
        {
            for (size_t j = 0; j < n_bytes; ++j)
            {
                INFO("j = " << j);
                REQUIRE(q_bytes[j] == v_bytes[j]);
            }
        }
        v_bytes += stride * sizeof(vole_block);
        q_bytes += stride * sizeof(vole_block);
    }
    for (size_t i = delta_bits; i < secpar_to_bits(S); ++i)
    {
        INFO("i = " << i);
        REQUIRE(delta_bytes[i] == 0);
        for (size_t j = 0; j < n_bytes; ++j)
        {
            {
                INFO("j = " << j);
                REQUIRE(v_bytes[j] == 0x00);
                REQUIRE(q_bytes[j] == 0x00);
            }
        }
        v_bytes += stride * sizeof(vole_block);
        q_bytes += stride * sizeof(vole_block);
    }
}

template <secpar S>
void inline verify_sub_field_vole(size_t n, const uint8_t* u, const block_secpar<S>* v,
                                  block_secpar<S> delta, const block_secpar<S>* q)
{
    for (size_t i = 0; i < n; ++i)
    {
        const auto u_i = (u[i / 8] >> (i % 8)) & 1;
        INFO("u_" << i << " = " << u_i);
        if (u_i)
            REQUIRE(q[i] == (v[i] ^ delta));
        else
            REQUIRE(q[i] == v[i]);
    }
}

template <secpar S>
void inline verify_whole_field_vole(size_t n, const block_secpar<S>* u, const block_secpar<S>* v,
                                    block_secpar<S> delta, const block_secpar<S>* q)
{
    const auto delta_p = poly_secpar<S>::load(&delta);
    for (size_t i = 0; i < n; ++i)
    {
        INFO("i = " << i);
        const auto u_p = poly_secpar<S>::load(&u[i]);
        const auto v_p = poly_secpar<S>::load(&v[i]);
        const auto q_p = poly_secpar<S>::load(&q[i]);
        const auto expected_q_p = (u_p * delta_p).template reduce_to<secpar_to_bits(S)>() + v_p;
        REQUIRE(q_p == expected_q_p);
    }
}

#endif
