#include <array>
#include <cstdint>

#include "prgs.hpp"
#include "test.hpp"
#include "test_prg_tvs.hpp"
#include "vector_com.inc"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("aes_ctr_prg", "[prg]", secpar128_t, secpar192_t, secpar256_t)
{
    constexpr auto S = TestType::value;
    using PRG_TVS = prg_tvs<secpar_to_bits(S)>;
    using PRG = aes_ctr_prg<S>;

    constexpr size_t num_keys = 1;
    constexpr size_t blocks_per_key = 3;

    std::array<typename PRG::block_t, num_keys * blocks_per_key> output;
    std::array<uint8_t, sizeof(output)> output_bytes;
    std::array<uint8_t, sizeof(output)> expected_output_bytes;
    std::array<typename PRG::expanded_key_t, num_keys> expanded_keys;
    static_assert(sizeof(output) <= sizeof(PRG_TVS::expected_output));
    std::array<typename PRG::key_t, num_keys> keys;
    typename PRG::iv_t iv;

    memcpy(&keys, PRG_TVS::key.data(), sizeof(keys));
    memcpy(&iv, PRG_TVS::iv.data(), sizeof(iv));
    PRG::template init<num_keys, blocks_per_key>(keys.data(), expanded_keys.data(), iv,
                                                 PRG_TVS::tweak, 0, output.data());

    memcpy(output_bytes.data(), output.data(), sizeof(expected_output_bytes));
    memcpy(expected_output_bytes.data(), PRG_TVS::expected_output.data(),
           sizeof(expected_output_bytes));

    REQUIRE(output_bytes == expected_output_bytes);

    PRG::template gen<num_keys, blocks_per_key>(expanded_keys.data(), iv,
                                                PRG_TVS::tweak, blocks_per_key, output.data());

    memcpy(output_bytes.data(), output.data(), sizeof(expected_output_bytes));
    memcpy(expected_output_bytes.data(),
           PRG_TVS::expected_output.data() + sizeof(expected_output_bytes),
           sizeof(expected_output_bytes));
    static_assert(sizeof(PRG_TVS::expected_output) >= 2 * sizeof(expected_output_bytes));

    REQUIRE(output_bytes == expected_output_bytes);
}

TEMPLATE_TEST_CASE("aes_ctr_expand_chunk", "[prg]", secpar128_t, secpar192_t, secpar256_t)
{
    constexpr auto S = TestType::value;
    using PRG_TVS = prg_tvs<secpar_to_bits(S)>;
    using PRG = aes_ctr_prg<S>;

    constexpr size_t num_keys = 1;
    constexpr size_t blocks_per_key = 4;

    std::array<block_secpar<S>, num_keys * blocks_per_key> output;
    std::array<uint8_t, sizeof(output)> output_bytes;
    std::array<uint8_t, sizeof(output)> expected_output_bytes;
    static_assert(sizeof(output) <= sizeof(PRG_TVS::expected_output));
    std::array<typename PRG::tweak_t, num_keys> tweaks = {PRG_TVS::tweak};
    std::array<typename PRG::key_t, num_keys> keys;
    typename PRG::iv_t iv;

    memcpy(&keys, PRG_TVS::key.data(), sizeof(keys));
    memcpy(&iv, PRG_TVS::iv.data(), sizeof(iv));
    expand_chunk<num_keys, 4, PRG>(iv, tweaks.data(), keys.data(), output.data());

    memcpy(output_bytes.data(), output.data(), sizeof(expected_output_bytes));
    memcpy(expected_output_bytes.data(), PRG_TVS::expected_output.data(),
           sizeof(expected_output_bytes));

    REQUIRE(output_bytes == expected_output_bytes);
}
