#include <array>
#include <iostream>
#include <type_traits>
#include <vector>

#include "crt_vole_helpers.inc"
#include "test.hpp"
#include "vole_commit.inc"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "test_vole_helpers.inc"

#include <print>

template <size_t DELTA_BITS, size_t LOOKUP_BITS> auto setup_mock_crt_columns_index_table()
{
    constexpr auto SPLIT = even_split(DELTA_BITS, LOOKUP_BITS);
    lift_columns_index_table_t<DELTA_BITS, DELTA_BITS, LOOKUP_BITS> crt_table = {};
    REQUIRE((SPLIT.N_MAX + SPLIT.N_MIN) == crt_table.size());
    REQUIRE(((SPLIT.N_MAX == 0) || (SPLIT.K_MAX <= LOOKUP_BITS)));
    REQUIRE(SPLIT.K_MIN <= LOOKUP_BITS);
    for (size_t chunk_l = 0; chunk_l < crt_table.size(); ++chunk_l)
    {
        const auto chunk_size = (chunk_l < SPLIT.N_MAX) ? SPLIT.K_MAX : SPLIT.K_MIN;
        REQUIRE(chunk_size <= LOOKUP_BITS);
        const auto mask = (chunk_size == 8) ? 0xff : ((1 << chunk_size) - 1);
        std::generate(crt_table[chunk_l].begin(), crt_table[chunk_l].end(),
                      [=] { return rand() & mask; });
    }
    return crt_table;
}

template <size_t DELTA_BITS, size_t LOOKUP_BITS> auto setup_mock_crt_row_lookup_table()
{
    lift_public_row_lookup_table_t<DELTA_BITS, DELTA_BITS, LOOKUP_BITS> crt_table = {};
    for (size_t i = 0; i < crt_table.size(); ++i)
    {
        for (size_t j = 0; j < crt_table[i].size(); ++j)
            std::generate(crt_table[i][j].begin(), crt_table[i][j].end(), [=] { return rand(); });
    }
    return crt_table;
};

TEMPLATE_TEST_CASE("bench_crt_lift_vole", "[.][bench][crt vole commit]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;
    using CP = P::CONSTS;
    constexpr auto S = P::secpar_v;
    constexpr auto DELTA_BITS = P::delta_bits_v;

    // Setup random input data
    auto data = vole_commit_data<P, false>::alloc();
    REQUIRE(data);
    {
        // fill v_columns with random data
        std::generate_n(data->v_columns().begin(), P::delta_bits_v * CP::VOLE_COL_BLOCKS,
                        [] { return rand_block<128>(); });
        // zero the columns corresponding to the zero bits in delta
        std::fill(data->v_columns().begin() + P::delta_bits_v * CP::VOLE_COL_BLOCKS,
                  data->v_columns().end(), vole_block::set_zero());

        // fill v with random data
        std::array<uint8_t, P::secpar_bytes> mask_bytes{};
        for (size_t i = 0; i < DELTA_BITS; ++i)
            mask_bytes[i / 8] |= 1 << (i % 8);
        block_secpar<S> mask;
        memcpy(&mask, mask_bytes.data(), sizeof(mask));
        std::generate(data->v().begin(), data->v().end(),
                      [=] { return rand_block<P::secpar_bits>() & mask; });
    }

    const auto bench_lift_columns = [&]<size_t LOOKUP_BITS, size_t BLOCKS_PER_ITERATION>
    {
        const auto crt_table = setup_mock_crt_columns_index_table<DELTA_BITS, LOOKUP_BITS>();
        BENCHMARK(std::format("crt_lift_columns<L={}, B={}> table_size={:.1f}KiB", LOOKUP_BITS,
                              BLOCKS_PER_ITERATION, sizeof(crt_table) / 1024.))
        {
            lift_columns_inplace<P::secpar_v, P::delta_bits_v, LOOKUP_BITS>(
                data->v_columns().data(), CP::VOLE_COL_BLOCKS, CP::VOLE_CORRECTION_BYTES,
                crt_table);
        };
    };

    const auto bench_lift_public_rows = [&]<size_t LOOKUP_BITS>
    {
        const auto crt_table = setup_mock_crt_row_lookup_table<DELTA_BITS, LOOKUP_BITS>();
        auto v = data->v();
        BENCHMARK(std::format("crt_lift_public_row<L={}> table_size={:.1f}KiB", LOOKUP_BITS,
                              sizeof(crt_table) / 1024.))
        {
            for (size_t i = 0; i < CP::VOLE_CORRECTION_ROWS; ++i)
                v[i] = lift_public_row<P::secpar_v, P::delta_bits_v, P::delta_bits_v, LOOKUP_BITS>(
                    v[i], crt_table);
        };
    };

    BENCHMARK("crt_lift_rows")
    {
        auto v = data->v();
        for (size_t i = 0; i < CP::VOLE_CORRECTION_ROWS; ++i)
            v[i] = crt_lift_row<P>(v[i]);
    };

    constexpr auto bench_for = []<size_t... Ns, typename F>(F&& f)
    { (f.template operator()<Ns>(), ...); };

    bench_for.template operator()<1, 2, 3, 4, 5, 6, 7, 8>(
        [&]<size_t L>()
        {
            bench_lift_columns.template operator()<L, 2>();
        });

    bench_for.template operator()<4, 8>([&]<size_t L>()
                                        { bench_lift_public_rows.template operator()<L>(); });
}

TEMPLATE_TEST_CASE("bench_tree_lift", "[.][bench][crt vole commit]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;
    constexpr auto S = P::secpar_v;
    const auto input = sample_delta<S>(P::delta_bits_v);

    BENCHMARK("tree_lift_row") { return tree_lift_row<P>(input); };

    BENCHMARK("tree_lift_public_row") { return tree_lift_public_row<P>(input); };
}

TEMPLATE_TEST_CASE("bench_gate_lift", "[.][bench][crt vole commit]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;
    using CP = P::CONSTS;
    std::array<uint8_t, CP::CRT_NUM_MULT_BYTES> input;
    std::generate(input.begin(), input.end(), rand<uint8_t>);

    BENCHMARK("gate_lift_row") { return gate_lift_row<P>(input.data()); };

    BENCHMARK("gate_lift_public_row") { return gate_lift_public_row<P>(input.data()); };
}

TEMPLATE_TEST_CASE("bench_G_lift", "[.][bench][crt vole commit]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;
    using CP = P::CONSTS;
    constexpr auto S = P::secpar_v;
    const auto input = sample_delta<S>(P::delta_bits_v);
    std::array<uint8_t, CP::CRT_NUM_MULT_BYTES> output{};

    BENCHMARK("G_lift_row_xor") { return G_lift_row_xor<P>(output.data(), input); };

    // BENCHMARK("G_lift_public_row_xor") { return G_lift_public_row_xor<P>(output.data(), input); };
}

#if 0
TEMPLATE_TEST_CASE("bench_crt_lift_columns", "[.][bench][crt vole commit]", ALL_FAEST_V2_INSTANCES)
{
    using P = TestType;
    using CP = P::CONSTS;
    constexpr auto S = P::secpar_v;
    constexpr auto DELTA_BITS = P::delta_bits_v;
    constexpr auto TAU = P::tau_v;

    constexpr size_t VOLE_BLOCKS = CP::VOLE_CORRECTION_BLOCKS;
    constexpr size_t TOTAL_BLOCKS = CP::VOLE_COL_BLOCKS;

    auto v = random_vector<vole_block>(P::secpar_bits * TOTAL_BLOCKS);
    // zero the columns corresponding to the zero bits in delta
    std::fill(v.begin() + P::delta_bits_v * TOTAL_BLOCKS, v.end(), vole_block::set_zero());

    const auto setup_mock_crt_table = [&]<size_t LOOKUP_BITS>()
    {
        size_t bits_remaining = DELTA_BITS;
        lift_columns_index_table_t<DELTA_BITS, DELTA_BITS, LOOKUP_BITS> crt_table = {};
        for (size_t chunk_l = 0; chunk_l < crt_table.size(); ++chunk_l)
        {
            const auto chunk_size = std::min(bits_remaining, LOOKUP_BITS);
            const auto mask = chunk_size == 8 ? 0xff : ((1 << chunk_size) - 1);
            std::generate(crt_table[chunk_l].begin(), crt_table[chunk_l].end(),
                          [=] { return rand() & mask; });
            bits_remaining -= LOOKUP_BITS;
        }
        return crt_table;
    };

    const auto bench = [&]<size_t LOOKUP_BITS, size_t BLOCKS_PER_ITERATION>
    {
        const auto crt_table = setup_mock_crt_table.template operator()<LOOKUP_BITS>();
        BENCHMARK(std::format("crt_lift_columns<L={}, B={}> table_size={:.1f}KiB", LOOKUP_BITS,
                              BLOCKS_PER_ITERATION, sizeof(crt_table) / 1024.))
        {
            crt_lift_v<P>>(
                v.data(), TOTAL_BLOCKS, VOLE_BLOCKS, crt_table);
        };
    };

    constexpr auto bench_for = []<size_t... Ns, typename F>(F&& f)
    { (f.template operator()<Ns>(), ...); };

    bench_for.template operator()<1, 2, 3, 4, 5, 6, 7, 8>(
        [&]<size_t L>()
        {
            bench_for.template operator()<1, 2, 4>([&]<size_t B>()
                                                   { bench.template operator()<L, B>(); });
        });
}

TEMPLATE_TEST_CASE("bench_crt_lift_public_row", "[.][bench][crt vole commit]",
                   ALL_FAEST_V2_INSTANCES)
{
    using P = TestType;
    constexpr auto S = P::secpar_v;
    constexpr auto DELTA_BITS = P::delta_bits_v;

    // generate delta
    const auto delta = sample_delta<S>(DELTA_BITS);

    const auto setup_mock_crt_table = [&]<size_t LOOKUP_BITS>()
    {
        lift_public_row_lookup_table_t<DELTA_BITS, DELTA_BITS, LOOKUP_BITS> crt_table = {};
        for (size_t i = 0; i < crt_table.size(); ++i)
        {
            for (size_t j = 0; j < crt_table[i].size(); ++j)
                std::generate(crt_table[i][j].begin(), crt_table[i][j].end(),
                              [=] { return rand(); });
        }
        return crt_table;
    };

    const auto bench = [&]<size_t LOOKUP_BITS>
    {
        const auto crt_table = setup_mock_crt_table.template operator()<LOOKUP_BITS>();
        BENCHMARK(std::format("crt_lift_public_row<L={}> table_size={:.1f}KiB", LOOKUP_BITS,
                              sizeof(crt_table) / 1024.))
        {
            return lift_public_row<S, DELTA_BITS, DELTA_BITS, LOOKUP_BITS>(delta, crt_table);
        };
    };

    bench.template operator()<4>();
    bench.template operator()<8>();
}

TEMPLATE_TEST_CASE("bench_crt_lift_row", "[.][bench][crt vole commit]",
                   ALL_FAEST_V2_INSTANCES)
{
    using P = TestType;
    constexpr auto S = P::secpar_v;
    // constexpr auto TAU = P::tau_v;
    constexpr auto DELTA_BITS = P::delta_bits_v;
    // using C = CRT_CONSTANTS<P::secpar_v, P::tau_v, P::delta_bits_v>;

    // generate delta
    const auto delta = sample_delta<S>(DELTA_BITS);

    BENCHMARK("crt_lift_row")
    {
        // return crt_lift_row<S, DELTA_BITS>(delta, CRT_CONSTANTS<S, TAU, DELTA_BITS>::W_CRT_T2);
        return crt_lift_row<P>(delta);
    };
}
#endif

TEMPLATE_TEST_CASE("bench_crt_vole_commit", "[.][bench][crt vole commit]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;
    using CP = CONSTANTS<P>;
    constexpr auto S = P::secpar_v;

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

    BENCHMARK("crt_vole_commit")
    {
        return crt_vole_commit<P>(seed, iv, *vole_com_data_sender,
                                  std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(
                                      commitment.begin(), commitment.end()),
                                  check_sender, c_mult);
    };

    hash_state_x4 hasher;
    hasher.init(S);
    uint32_t counter = 0;
    auto delta = block_secpar<S>::set_zero();
    BENCHMARK("gind_and_open")
    {
        return grind_and_open<typename P::bavc_t>(
            vole_com_data_sender->forest().data(), vole_com_data_sender->hashed_leaves().data(),
            reinterpret_cast<uint8_t*>(&delta), opening.data(), &hasher, &counter);
    };

    BENCHMARK("crt_vole_reconstruct")
    {
        return crt_vole_reconstruct<P>(
            iv, *vole_com_data_receiver, delta,
            std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(),
                                                                 commitment.end()),
            std::span<uint8_t, P::bavc_t::OPEN_SIZE>(opening.begin(), opening.end()),
            check_receiver, c_mult);
    };
    // const auto crt_delta = crt_lift_delta<P>(delta);
}

TEMPLATE_TEST_CASE("bench_classic_vole_commit_and_transpose", "[.][bench][crt vole commit]",
                   ALL_FAEST_V2_INSTANCES)
{
    using P = TestType;
    using CP = CONSTANTS<P>;
    constexpr auto S = P::secpar_v;

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

    BENCHMARK("vole_commit_and_transpose")
    {
        vole_commit<P>(seed, iv, *vole_com_data_sender,
                       std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(),
                                                                            commitment.end()),
                       check_sender);
        transpose_secpar<S>(vole_com_data_sender->v_columns().data(),
                            vole_com_data_sender->v().data(), CP::VOLE_COL_STRIDE,
                            CP::QUICKSILVER_ROWS_PADDED);
    };

    hash_state_x4 hasher;
    hasher.init(S);
    uint32_t counter = 0;
    auto delta = block_secpar<S>::set_zero();
    BENCHMARK("grind_and_open")
    {
        return grind_and_open<typename P::bavc_t>(
            vole_com_data_sender->forest().data(), vole_com_data_sender->hashed_leaves().data(),
            reinterpret_cast<uint8_t*>(&delta), opening.data(), &hasher, &counter);
    };
    std::vector<uint8_t> delta_bytes(secpar_to_bits(S), 0);
    expand_bits_to_bytes(delta_bytes.data(), P::delta_bits_v,
                         reinterpret_cast<const uint8_t*>(&delta));
    BENCHMARK("vole_reconstruct_and_transpose")
    {
        bool vole_reconstruct_successful = vole_reconstruct<P>(
            iv, *vole_com_data_receiver, delta_bytes.data(),
            std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(),
                                                                 commitment.end()),
            std::span<uint8_t, P::bavc_t::OPEN_SIZE>(opening.begin(), opening.end()),
            check_receiver);
        transpose_secpar<S>(vole_com_data_receiver->q_columns().data(),
                            vole_com_data_receiver->q().data(), CP::VOLE_COL_STRIDE,
                            CP::QUICKSILVER_ROWS_PADDED);
        return vole_reconstruct_successful;
    };
}
