#include <array>
#include <type_traits>
#include <vector>

#include "crt_vole_helpers.inc"
#include "test.hpp"
#include "test_crt_vole_commit_tvs.hpp"
#include "test_vole_commit_tvs_v3.hpp"
#include "transpose_secpar.hpp"
#include "vole_commit.inc"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "test_vole_helpers.inc"

TEMPLATE_TEST_CASE("crt_lift_delta", "[crt vole commit]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;
    constexpr auto S = P::secpar_v;

    const auto& [delta_bytes, expected_crt_delta_bytes] = []
    {
        if constexpr (std::is_same_v<P, v3::faest_128_s>)
            return std::make_pair(crt_tvs_128_s::delta_bytes,
                                  crt_tvs_128_s::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_128_f>)
            return std::make_pair(crt_tvs_128_f::delta_bytes,
                                  crt_tvs_128_f::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_192_s>)
            return std::make_pair(crt_tvs_192_s::delta_bytes,
                                  crt_tvs_192_s::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_192_f>)
            return std::make_pair(crt_tvs_192_f::delta_bytes,
                                  crt_tvs_192_f::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_256_s>)
            return std::make_pair(crt_tvs_256_s::delta_bytes,
                                  crt_tvs_256_s::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_256_f>)
            return std::make_pair(crt_tvs_256_f::delta_bytes,
                                  crt_tvs_256_f::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_em_128_s>)
            return std::make_pair(crt_tvs_em_128_s::delta_bytes,
                                  crt_tvs_em_128_s::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_em_128_f>)
            return std::make_pair(crt_tvs_em_128_f::delta_bytes,
                                  crt_tvs_em_128_f::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_em_192_s>)
            return std::make_pair(crt_tvs_em_192_s::delta_bytes,
                                  crt_tvs_em_192_s::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_em_192_f>)
            return std::make_pair(crt_tvs_em_192_f::delta_bytes,
                                  crt_tvs_em_192_f::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_em_256_s>)
            return std::make_pair(crt_tvs_em_256_s::delta_bytes,
                                  crt_tvs_em_256_s::expected_crt_delta_bytes);
        else if constexpr (std::is_same_v<P, v3::faest_em_256_f>)
            return std::make_pair(crt_tvs_em_256_f::delta_bytes,
                                  crt_tvs_em_256_f::expected_crt_delta_bytes);
    }();

    std::array<uint8_t, secpar_to_bytes(S)> crt_delta_bytes;
    auto delta = block_secpar<S>::set_zero();
    memcpy(&delta, delta_bytes.data(), sizeof(delta));
    static_assert(sizeof(delta) == sizeof(delta_bytes));
    {
        const auto crt_delta = crt_lift_delta<P>(delta);
        memcpy(crt_delta_bytes.data(), &crt_delta, sizeof(crt_delta_bytes));
        REQUIRE(crt_delta_bytes == expected_crt_delta_bytes);
    }
    // crt_lift_delta will be implemented with one of the follow function, so we test all of them.
    {
        const auto crt_delta = crt_lift_public_row<P>(delta);
        memcpy(crt_delta_bytes.data(), &crt_delta, sizeof(crt_delta_bytes));
        REQUIRE(crt_delta_bytes == expected_crt_delta_bytes);
    }
    {
        const auto crt_delta = crt_lift_row<P>(delta);
        memcpy(crt_delta_bytes.data(), &crt_delta, sizeof(crt_delta_bytes));
        REQUIRE(crt_delta_bytes == expected_crt_delta_bytes);
    }
}

#if 0
using two = std::integral_constant<size_t, 2>;
using four = std::integral_constant<size_t, 5>;
using eight = std::integral_constant<size_t, 8>;
TEMPLATE_TEST_CASE("build_gray_table", "[crt vole commit]", two, four, eight)
{
    constexpr size_t LOOKUP_BITS = TestType::value;
    constexpr size_t TABLE_SIZE = 1 << LOOKUP_BITS;

    constexpr size_t VOLE_BLOCKS = 2;
    constexpr size_t PADDING_BLOCKS = 1;
    constexpr size_t BLOCKS_PER_ITERATION = 2;
    constexpr size_t TOTAL_BLOCKS = VOLE_BLOCKS + PADDING_BLOCKS;

    constexpr auto gray_tables = build_gray_bitflip_table<LOOKUP_BITS>();
    std::array<std::array<vole_block, BLOCKS_PER_ITERATION>, TABLE_SIZE> table;

    const auto inputs = random_vector<vole_block>(LOOKUP_BITS * TOTAL_BLOCKS);
    {
        build_gray_table<vole_block, BLOCKS_PER_ITERATION, LOOKUP_BITS>(table, inputs.data(),
                                                                        TOTAL_BLOCKS, gray_tables);

        for (size_t i = 0; i < TABLE_SIZE; ++i)
        {
            for (size_t c = 0; c < BLOCKS_PER_ITERATION; ++c)
            {
                // std::cerr << i << ": " << table[i][c] << '\n';
                auto expected_entry = vole_block::set_zero();
                for (size_t bit_j = 0; bit_j < LOOKUP_BITS; ++bit_j)
                {
                    if ((i >> bit_j) & 1)
                        expected_entry = expected_entry ^ inputs[bit_j * TOTAL_BLOCKS + c];
                }
                INFO("i = " << i);
                REQUIRE(table[i][c] == expected_entry);
            }
        }
    }
    // Now check a table that is 1 bit smaller with the same codewords/indices
    {
        build_gray_table<vole_block, BLOCKS_PER_ITERATION, LOOKUP_BITS - 1, LOOKUP_BITS>(
            table, inputs.data(), TOTAL_BLOCKS, gray_tables);

        for (size_t i = 0; i < TABLE_SIZE / 2; ++i)
        {
            for (size_t c = 0; c < BLOCKS_PER_ITERATION; ++c)
            {
                // std::cerr << i << ": " << table[i][c] << '\n';
                auto expected_entry = vole_block::set_zero();
                for (size_t bit_j = 0; bit_j < LOOKUP_BITS - 1; ++bit_j)
                {
                    if ((i >> bit_j) & 1)
                        expected_entry = expected_entry ^ inputs[bit_j * TOTAL_BLOCKS + c];
                }
                INFO("i = " << i);
                REQUIRE(table[i][c] == expected_entry);
            }
        }
    }
}

TEMPLATE_TEST_CASE("crt_lift_vole", "[crt vole commit]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;
    constexpr auto S = P::secpar_v;
    constexpr auto DELTA_BITS = P::delta_bits_v;

    // constexpr size_t VOLE_BLOCKS = 4;
    // constexpr size_t PADDING_BLOCKS = 3;
    constexpr size_t VOLE_BLOCKS = 3;
    constexpr size_t PADDING_BLOCKS = 0;
    constexpr size_t TOTAL_BLOCKS = VOLE_BLOCKS + PADDING_BLOCKS;

    auto [u, v, delta, q] = gen_vole_columns<S, DELTA_BITS>(TOTAL_BLOCKS);
    {
        INFO("checking original VOLE correlation");
        INFO("delta = " << delta);
        verify_vole_columns<S>(DELTA_BITS, TOTAL_BLOCKS * sizeof(vole_block), TOTAL_BLOCKS,
                               u.data(), v.data(), delta, q.data());
    }

    // apply CRT lift to v, q, Delta
    crt_lift_v<P>(v.data(), TOTAL_BLOCKS, VOLE_BLOCKS);
    crt_lift_q<P>(q.data(), TOTAL_BLOCKS, VOLE_BLOCKS);
    const auto crt_delta = crt_lift_delta<P>(delta);

    // sanity check that lifted delta is non-zero and different from the original
    REQUIRE(crt_delta != block_secpar<S>::set_zero());
    REQUIRE(crt_delta != delta);

    // check that VOLE correlation still holds
    {
        INFO("checking lifted VOLE correlation");
        INFO("crt_delta = " << crt_delta);
        verify_vole_columns<S>(DELTA_BITS, VOLE_BLOCKS * sizeof(vole_block), TOTAL_BLOCKS, u.data(),
                               v.data(), crt_delta, q.data());
    }
}

#endif

TEMPLATE_TEST_CASE("crt commit/open/verify", "[vole commit]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;
    using CP = CONSTANTS<P>;
    constexpr auto S = P::secpar_v;
    constexpr auto N_MASK = CP::CRT_NUM_MASK;

    block_secpar<S> seed = rand<block_secpar<S>>();
    block128 iv = rand<block128>();
    auto vole_com_data_sender = vole_commit_data<P, false>::alloc();
    auto vole_com_data_receiver = vole_commit_data<P, true>::alloc();
    REQUIRE(vole_com_data_sender);
    REQUIRE(vole_com_data_receiver);
    std::vector<uint8_t> commitment(CP::VOLE_COMMIT_CORRECTIONS_SIZE);
    std::vector<uint8_t> opening(P::bavc_t::OPEN_SIZE);
    std::array<uint8_t, CP::VOLE_COMMIT_CHECK_SIZE> check_sender;
    std::array<uint8_t, CP::VOLE_COMMIT_CHECK_SIZE> check_receiver;
    std::array<uint8_t, CP::CRT_CMULT_SIZE_BYTES> c_mult;

    crt_vole_commit<P>(
        seed, iv, *vole_com_data_sender,
        std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(), commitment.end()),
        check_sender, c_mult);

    hash_state_x4 hasher;
    hasher.init(S);
    uint32_t counter = 0;
    auto delta = block_secpar<S>::set_zero();

    bool bavc_open_successful = grind_and_open<typename P::bavc_t>(
        vole_com_data_sender->forest().data(), vole_com_data_sender->hashed_leaves().data(),
        reinterpret_cast<uint8_t*>(&delta), opening.data(), &hasher, &counter);
    REQUIRE(bavc_open_successful);
    bool vole_reconstruct_successful = crt_vole_reconstruct<P>(
        iv, *vole_com_data_receiver, delta,
        std::span<const uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(),
                                                                   commitment.end()),
        std::span<const uint8_t, P::bavc_t::OPEN_SIZE>(opening.begin(), opening.end()),
        check_receiver, c_mult);
    const auto crt_delta = crt_lift_delta<P>(delta);
    REQUIRE(vole_reconstruct_successful);
    REQUIRE(check_receiver == check_sender);
    {
        INFO("check that VOLE correlation of (u, v, Delta, q) holds (column-wise)");
        INFO("delta = " << delta);
        INFO("crt_delta = " << crt_delta);
        verify_vole_columns<S>(P::delta_bits_v, CP::VOLE_CORRECTION_BYTES, CP::VOLE_COL_BLOCKS,
                               vole_com_data_sender->u().data(),
                               vole_com_data_sender->v_columns().data(), crt_delta,
                               vole_com_data_receiver->q_columns().data());
    }
    {
        INFO("check that VOLE correlation of (u, V, Delta, Q) holds (row-wise)");
        INFO("crt_delta = " << crt_delta);
        verify_sub_field_vole<S>(CP::VOLE_CORRECTION_ROWS,
                                 reinterpret_cast<const uint8_t*>(vole_com_data_sender->u().data()),
                                 vole_com_data_sender->v().data(), crt_delta,
                                 vole_com_data_receiver->q().data());
    }

    {
        INFO("check that VOLE correlation of (u_mask, v_mask, Delta, q_mask) holds");
        verify_whole_field_vole<S>(N_MASK, vole_com_data_sender->u_mask().data(),
                                   vole_com_data_sender->v_mask().data(), crt_delta,
                                   vole_com_data_receiver->q_mask().data());
    }
}

TEMPLATE_TEST_CASE("commit test vectors v3", "[vole commit]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;
    using CP = CONSTANTS<P>;
    constexpr auto S = P::secpar_v;
    constexpr auto N_MASK = CP::CRT_NUM_MASK;
    using TVS = test_vole_commit_tvs_v3::vole_commit_v3_tvs<P>;

    REQUIRE(CP::VOLE_CORRECTION_ROWS == TVS::ell);
    REQUIRE(CP::CRT_NUM_MASK == TVS::nmask);
    REQUIRE(CP::CRT_NUM_MULT == TVS::crtmults);

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

    auto vole_com_data_sender = vole_commit_data<P, false>::alloc();
    auto vole_com_data_receiver = vole_commit_data<P, true>::alloc();
    std::vector<uint8_t> commitment(CP::VOLE_COMMIT_CORRECTIONS_SIZE);
    std::vector<uint8_t> opening(P::bavc_t::OPEN_SIZE);
    std::array<uint8_t, CP::VOLE_COMMIT_CHECK_SIZE> check_sender;
    std::array<uint8_t, CP::VOLE_COMMIT_CHECK_SIZE> check_receiver;
    std::array<uint8_t, CP::CRT_CMULT_SIZE_BYTES> c_mult;

    // commit
    crt_vole_commit<P>(
        seed, iv, *vole_com_data_sender,
        std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(), commitment.end()),
        check_sender, c_mult);

    static_assert(CP::VOLE_COL_BLOCKS >=
                  (CP::VOLE_ROWS + 8 * sizeof(vole_block) - 1) / sizeof(vole_block) / 8);
    const auto hashed_u = hash_buf(vole_com_data_sender->u().data(), CP::VOLE_CORRECTION_BYTES);
    CHECK(hashed_u == TVS::hashed_u);

    const auto hashed_u_mask =
        hash_buf(vole_com_data_sender->u_mask().data(), N_MASK * P::secpar_bytes);
    CHECK(hashed_u_mask == TVS::hashed_barU);

    const auto hashed_commitment =
        hash_buf(commitment.data(), commitment.size());
    CHECK(hashed_commitment == TVS::hashed_c);

    hash_state hasher;
    hasher.init(secpar::s256);
    hasher.update(vole_com_data_sender->v().data(), P::secpar_bytes * CP::VOLE_CORRECTION_ROWS);
    std::array<uint8_t, 64> hashed_v;
    hasher.finalize(hashed_v.data(), hashed_v.size());
    CHECK(hashed_v == TVS::hashed_mV);

    const auto hashed_v_mask =
        hash_buf(vole_com_data_sender->v_mask().data(), N_MASK * P::secpar_bytes);
    CHECK(hashed_v_mask == TVS::hashed_barV);

    const auto hashed_c_mult = hash_buf(c_mult.data(), c_mult.size());
    CHECK(hashed_c_mult == TVS::hashed_cmult);

    CHECK(check_sender == TVS::com);

    // open
    block_secpar<S> delta;
    memcpy(&delta, TVS::chall.data(), sizeof(delta));
    std::array<uint8_t, P::bavc_t::delta_bits_v> delta_bytes = {0};
    for (size_t i = 0; i < delta_bytes.size(); ++i)
    {
        delta_bytes[i] = ((TVS::chall[i / 8] >> (i % 8)) & 1) ? 0xff : 0x00;
    }
    bool bavc_open_successful = P::bavc_t::open(vole_com_data_sender->forest().data(),
                                                vole_com_data_sender->hashed_leaves().data(),
                                                delta_bytes.data(), opening.data());
    REQUIRE(bavc_open_successful);

    // reconstruct
    bool vole_reconstruct_successful = crt_vole_reconstruct<P>(
        iv, *vole_com_data_receiver, delta,
        std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(), commitment.end()),
        std::span<uint8_t, P::bavc_t::OPEN_SIZE>(opening.begin(), opening.end()), check_receiver,
        c_mult);

    REQUIRE(vole_reconstruct_successful);
    REQUIRE(check_receiver == check_sender);
    hasher.init(secpar::s256);
    hasher.update(vole_com_data_receiver->q().data(), P::secpar_bytes * CP::VOLE_CORRECTION_ROWS);

    std::array<uint8_t, 64> hashed_q;
    hasher.finalize(hashed_q.data(), hashed_q.size());
    CHECK(hashed_q == TVS::hashed_mQ);

    const auto hashed_q_mask =
        hash_buf(vole_com_data_receiver->q_mask().data(), N_MASK * P::secpar_bytes);
    CHECK(hashed_q_mask == TVS::hashed_barQ);
}
