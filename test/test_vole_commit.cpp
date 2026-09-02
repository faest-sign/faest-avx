#include <array>
#include <type_traits>
#include <vector>

#include "test.hpp"
#include "test_vole_commit_tvs_v2.hpp"
#include "vector_com.inc"
#include "vole_commit.inc"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "test_vole_helpers.inc"

TEMPLATE_TEST_CASE("commit/open/verify", "[vole commit]", ALL_FAEST_V2_INSTANCES)
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

    vole_commit<P>(
        seed, iv, *vole_com_data_sender,
        std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(), commitment.end()),
        check_sender);

    hash_state_x4 hasher;
    hasher.init(S);
    uint32_t counter = 0;
    auto delta = block_secpar<S>::set_zero();

    bool bavc_open_successful = grind_and_open<typename P::bavc_t>(
        vole_com_data_sender->forest().data(), vole_com_data_sender->hashed_leaves().data(),
        reinterpret_cast<uint8_t*>(&delta), opening.data(), &hasher, &counter);
    REQUIRE(bavc_open_successful);
    std::vector<uint8_t> delta_bytes(secpar_to_bits(S), 0);
    expand_bits_to_bytes(delta_bytes.data(), P::delta_bits_v,
                         reinterpret_cast<const uint8_t*>(&delta));
    bool vole_reconstruct_successful = vole_reconstruct<P>(
        iv, *vole_com_data_receiver, delta_bytes.data(),
        std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(), commitment.end()),
        std::span<uint8_t, P::bavc_t::OPEN_SIZE>(opening.begin(), opening.end()), check_receiver);
    REQUIRE(vole_reconstruct_successful);
    REQUIRE(check_receiver == check_sender);
    verify_vole_columns<S>(P::delta_bits_v, CP::VOLE_BYTES, CP::VOLE_COL_BLOCKS,
                           vole_com_data_sender->u().data(),
                           vole_com_data_sender->v_columns().data(), delta,
                           vole_com_data_receiver->q_columns().data());
}

TEMPLATE_TEST_CASE("commit test vectors v2", "[vole commit]", ALL_FAEST_V2_INSTANCES)
{
    using P = TestType;
    using CP = CONSTANTS<P>;
    constexpr auto S = P::secpar_v;
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

    auto vole_com_data_sender = vole_commit_data<P, false>::alloc();
    auto vole_com_data_receiver = vole_commit_data<P, true>::alloc();
    REQUIRE(vole_com_data_sender);
    REQUIRE(vole_com_data_receiver);
    std::vector<uint8_t> commitment(CP::VOLE_COMMIT_CORRECTIONS_SIZE);
    std::vector<uint8_t> opening(P::bavc_t::OPEN_SIZE);
    std::array<uint8_t, CP::VOLE_COMMIT_CHECK_SIZE> check_sender;
    std::array<uint8_t, CP::VOLE_COMMIT_CHECK_SIZE> check_receiver;

    // commit
    vole_commit<P>(
        seed, iv, *vole_com_data_sender,
        std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(), commitment.end()),
        check_sender);

    static_assert(CP::VOLE_COL_BLOCKS >=
                  (CP::VOLE_ROWS + 8 * sizeof(vole_block) - 1) / sizeof(vole_block) / 8);
    const auto hashed_u = hash_buf(vole_com_data_sender->u().data(), CP::VOLE_ROWS / 8);
    CHECK(hashed_u == TVS::hashed_u);

    const auto hashed_commitment =
        hash_buf(commitment.data(), commitment.size() * sizeof(commitment[0]));
    CHECK(hashed_commitment == TVS::hashed_c);

    hash_state hasher;
    hasher.init(secpar::s256);
    for (size_t i = 0; i < P::secpar_bits; ++i)
    {
        hasher.update(vole_com_data_sender->v_columns().data() + i * CP::VOLE_COL_BLOCKS,
                      CP::VOLE_ROWS / 8);
    }
    std::array<uint8_t, 64> hashed_v;
    hasher.finalize(hashed_v.data(), hashed_v.size());
    CHECK(hashed_v == TVS::hashed_v);

    CHECK(check_sender == TVS::h);

    // open
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
    bool vole_reconstruct_successful = vole_reconstruct<P>(
        iv, *vole_com_data_receiver, delta_bytes.data(),
        std::span<uint8_t, CP::VOLE_COMMIT_CORRECTIONS_SIZE>(commitment.begin(), commitment.end()),
        std::span<uint8_t, P::bavc_t::OPEN_SIZE>(opening.begin(), opening.end()), check_receiver);
    REQUIRE(vole_reconstruct_successful);

    REQUIRE(check_receiver == check_sender);
    hasher.init(secpar::s256);
    for (size_t i = 0; i < P::secpar_bits; ++i)
    {
        hasher.update(vole_com_data_receiver->q_columns().data() + i * CP::VOLE_COL_BLOCKS,
                      CP::VOLE_ROWS / 8);
    }
    std::array<uint8_t, 64> hashed_q;
    hasher.finalize(hashed_q.data(), hashed_q.size());
    CHECK(hashed_q == TVS::hashed_q);
}
