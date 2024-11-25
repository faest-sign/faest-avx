#include <array>

#include "owf_proof_tools.hpp"
#include "quicksilver.hpp"
#include "test.hpp"
#include "test_gfsmall_tvs.hpp"
#include "test_aes_tvs.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>


TEMPLATE_TEST_CASE("gf256_square", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    using test_state = quicksilver_test_state<TestType::value>;
    using QSP = test_state::QSP;
    using QSV = test_state::QSV;
    constexpr auto S = TestType::value;

    const auto delta = rand<block_secpar<S>>();
    test_state qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    auto x_in_p = qs_state_prover.template const_gf2_array<8>();
    auto x_in_v = qs_state_verifier.template const_gf2_array<8>();
    auto x_sq_p = qs_state_prover.template const_gf2_array<8>();
    auto x_sq_v = qs_state_verifier.template const_gf2_array<8>();

    // x = W^7 + W^5 + W^2 + 1
    const std::array<poly1, 8> x = {
        poly1::set_one(),  poly1::set_zero(), poly1::set_one(),  poly1::set_zero(),
        poly1::set_zero(), poly1::set_one(),  poly1::set_zero(), poly1::set_one(),
    };
    // y = W^7 + W^6 + W^5 + W^2 + W + 1
    const std::array<poly1, 8> y = {
        poly1::set_one(),  poly1::set_one(), poly1::set_one(), poly1::set_zero(),
        poly1::set_zero(), poly1::set_one(), poly1::set_one(), poly1::set_one(),
    };

    for (size_t i = 0; i < 8; ++i)
    {
        x_in_p[i] = quicksilver_gf2<QSP>(x[i], &qs_state_prover);
        x_in_v[i] = quicksilver_gf2<QSV>(x[i], &qs_state_verifier);
        REQUIRE(x_in_p[i].value() == x[i]);
    }
    square_8_bits(x_sq_p.data(), x_in_p.data());
    square_8_bits(x_sq_v.data(), x_in_v.data());
    for (size_t i = 0; i < 8; ++i)
    {
        REQUIRE(x_sq_p[i].value() == y[i]);
        REQUIRE(qs_test.check_mac(x_sq_p[i], x_sq_v[i]));
    }
}

TEMPLATE_TEST_CASE("gf256_decompress_gf16_subfield", "[quicksilver]", secpar128_t, secpar192_t,
                   secpar256_t)
{
    using test_state = quicksilver_test_state<TestType::value>;
    using QSP = test_state::QSP;
    using QSV = test_state::QSV;
    constexpr auto S = TestType::value;

    const auto delta = rand<block_secpar<S>>();
    test_state qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    auto x_p = qs_state_prover.template const_gf2_array<4>();
    auto x_v = qs_state_verifier.template const_gf2_array<4>();
    auto y_p = qs_state_prover.template const_gf2_array<8>();
    auto y_v = qs_state_verifier.template const_gf2_array<8>();

    for (size_t j = 0; j < GF16_SUBFIELD_ELEMENTS_COMPRESSED.size(); ++j)
    {
        const auto x = GF16_SUBFIELD_ELEMENTS_COMPRESSED[j];
        const auto y = GF16_SUBFIELD_ELEMENTS[j];
        for (size_t bit_i = 0; bit_i < 4; ++bit_i)
        {
            x_p[bit_i] = quicksilver_gf2<QSP>(poly1::load(x, bit_i), &qs_state_prover);
            x_v[bit_i] = quicksilver_gf2<QSV>(poly1::load(x, bit_i), &qs_state_verifier);
        }
        decompress_gf16_subfield(y_p, x_p);
        decompress_gf16_subfield(y_v, x_v);
        for (size_t bit_i = 0; bit_i < 8; ++bit_i)
        {
            REQUIRE(y_p[bit_i].value() == poly1::load(y, bit_i));
            REQUIRE(qs_test.check_mac(y_p[bit_i], y_v[bit_i]));
        }
    }
}

TEMPLATE_TEST_CASE("gf256_g2_conjugates", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    using test_state = quicksilver_test_state<TestType::value>;
    using QSP = test_state::QSP;
    using QSV = test_state::QSV;
    constexpr auto S = TestType::value;

    const auto delta = rand<block_secpar<S>>();
    test_state qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    auto x_in_p = qs_state_prover.template const_gf2_array<8>();
    auto x_in_v = qs_state_verifier.template const_gf2_array<8>();
    auto conjugates_p = qs_state_prover.template const_gfsecpar_array<8>();
    auto conjugates_v = qs_state_verifier.template const_gfsecpar_array<8>();

    // x = W^7 + W^5 + W^2 + 1
    const std::array<poly1, 8> x = {
        poly1::set_one(),  poly1::set_zero(), poly1::set_one(),  poly1::set_zero(),
        poly1::set_zero(), poly1::set_one(),  poly1::set_zero(), poly1::set_one(),
    };
    const auto x_in_gfsecpar = poly<secpar_to_bits(S)>::from_8_poly1(x.data());

    for (size_t i = 0; i < 8; ++i)
    {
        x_in_p[i] = quicksilver_gf2<QSP>(x[i], &qs_state_prover);
        x_in_v[i] = quicksilver_gf2<QSV>(x[i], &qs_state_verifier);
        REQUIRE(x_in_p[i].value() == x[i]);
        REQUIRE(qs_test.check_mac(x_in_p[i], x_in_v[i]));
    }
    gf256_gf2_conjugates<8>(&qs_state_prover, conjugates_p, x_in_p);
    gf256_gf2_conjugates<8>(&qs_state_verifier, conjugates_v, x_in_v);
    REQUIRE(conjugates_p[0].value() == x_in_gfsecpar);
    REQUIRE(qs_test.check_mac(conjugates_p[0], conjugates_v[0]));
    for (size_t j = 1; j < 8; ++j)
    {
        REQUIRE(conjugates_p[j].value() ==
                (conjugates_p[j - 1].value() * conjugates_p[j - 1].value())
                    .template reduce_to<secpar_to_bits(S)>());
        REQUIRE(qs_test.check_mac(conjugates_p[j], conjugates_v[j]));
    }
}

TEMPLATE_TEST_CASE("aes_mix_columns_and_add_roundkey_inplace", "[quicksilver]", secpar128_t,
                   secpar192_t, secpar256_t)
{
    constexpr auto S = TestType::value;
    using test_state = quicksilver_test_state<S>;
    using QSP = test_state::QSP;
    using QSV = test_state::QSV;
    constexpr auto O = owf::aes_ecb;
    using OC = OWF_CONSTANTS<S, O>;
    using AES_TVS = aes_tvs<secpar_to_bits(S)>;

    const auto delta = rand<block_secpar<S>>();
    test_state qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    auto state_p = qs_state_prover.template const_gfsecpar_array<OC::OWF_BLOCK_SIZE>();
    auto state_v = qs_state_verifier.template const_gfsecpar_array<OC::OWF_BLOCK_SIZE>();
    auto state_sq_p = qs_state_prover.template const_gfsecpar_array<OC::OWF_BLOCK_SIZE>();
    auto state_sq_v = qs_state_verifier.template const_gfsecpar_array<OC::OWF_BLOCK_SIZE>();

    auto rk_p = qs_state_prover.template const_gfsecpar_array<OC::OWF_BLOCK_SIZE>();
    auto rk_v = qs_state_verifier.template const_gfsecpar_array<OC::OWF_BLOCK_SIZE>();
    auto rk_sq_p = qs_state_prover.template const_gfsecpar_array<OC::OWF_BLOCK_SIZE>();
    auto rk_sq_v = qs_state_verifier.template const_gfsecpar_array<OC::OWF_BLOCK_SIZE>();

    for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
    {
        const auto byte = poly_secpar<S>::from_8_byte(static_cast<uint8_t>(byte_j));
        const auto byte_sq = (byte * byte).template reduce_to<secpar_to_bits(S)>();
        rk_p[byte_j] = quicksilver_gfsecpar<QSP>(byte, &qs_state_prover);
        rk_v[byte_j] = quicksilver_gfsecpar<QSV>(byte, &qs_state_verifier);
        rk_sq_p[byte_j] = quicksilver_gfsecpar<QSP>(byte_sq, &qs_state_prover);
        rk_sq_v[byte_j] = quicksilver_gfsecpar<QSV>(byte_sq, &qs_state_verifier);
    }

    for (size_t i = 0; i < AES_TVS::n; ++i)
    {
        for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
        {
            const auto byte = poly_secpar<S>::from_8_byte(AES_TVS::AFTER_SHIFT_ROWS[i][byte_j]);
            const auto byte_sq = (byte * byte).template reduce_to<secpar_to_bits(S)>();
            state_p[byte_j] = quicksilver_gfsecpar<QSP>(byte, &qs_state_prover);
            state_v[byte_j] = quicksilver_gfsecpar<QSV>(byte, &qs_state_verifier);
            state_sq_p[byte_j] = quicksilver_gfsecpar<QSP>(byte_sq, &qs_state_prover);
            state_sq_v[byte_j] = quicksilver_gfsecpar<QSV>(byte_sq, &qs_state_verifier);
        }
        mix_columns_and_add_roundkey_inplace<S, O, false>(&qs_state_prover, state_p, rk_p.data());
        mix_columns_and_add_roundkey_inplace<S, O, false>(&qs_state_verifier, state_v, rk_v.data());
        mix_columns_and_add_roundkey_inplace<S, O, true>(&qs_state_prover, state_sq_p,
                                                         rk_sq_p.data());
        mix_columns_and_add_roundkey_inplace<S, O, true>(&qs_state_verifier, state_sq_v,
                                                         rk_sq_v.data());
        for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
        {

            const auto expected_byte = poly_secpar<S>::from_8_byte(
                AES_TVS::AFTER_MIX_COLUMNS[i][byte_j] ^ static_cast<uint8_t>(byte_j));
            const auto expected_byte_sq =
                (expected_byte * expected_byte).template reduce_to<secpar_to_bits(S)>();
            REQUIRE(state_p[byte_j].value() == expected_byte);
            REQUIRE(state_sq_p[byte_j].value() == expected_byte_sq);
            REQUIRE(qs_test.check_mac(state_p[byte_j], state_v[byte_j]));
            REQUIRE(qs_test.check_mac(state_sq_p[byte_j], state_sq_v[byte_j]));
        }
    }
}

TEMPLATE_TEST_CASE("aes_sbox_affine", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    constexpr auto S = TestType::value;
    using test_state = quicksilver_test_state<S>;
    using QSP = test_state::QSP;
    using QSV = test_state::QSV;
    constexpr auto O = owf::aes_ecb;
    using OC = OWF_CONSTANTS<S, O>;
    using AES_TVS = aes_tvs<secpar_to_bits(S)>;

    const auto delta = rand<block_secpar<S>>();
    test_state qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    auto state_p = qs_state_prover.template const_gf2_2d_array<8, OC::OWF_BLOCK_SIZE>();
    auto state_v = qs_state_verifier.template const_gf2_2d_array<8, OC::OWF_BLOCK_SIZE>();
    auto state_with_conjugates_p =
        qs_state_prover.template const_gfsecpar_2d_array<8, OC::OWF_BLOCK_SIZE>();
    auto state_with_conjugates_v =
        qs_state_verifier.template const_gfsecpar_2d_array<8, OC::OWF_BLOCK_SIZE>();
    auto out_state_with_sq_p =
        qs_state_prover.template const_gfsecpar_2d_array<OC::OWF_BLOCK_SIZE, 2>();
    auto out_state_with_sq_v =
        qs_state_verifier.template const_gfsecpar_2d_array<OC::OWF_BLOCK_SIZE, 2>();
    static_assert(state_with_conjugates_p.size() == OC::OWF_BLOCK_SIZE);
    static_assert(state_with_conjugates_p[0].size() == 8);

    for (size_t i = 0; i < AES_TVS::n; ++i)
    {
        // prepare input state
        for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
        {
            const auto byte = AES_TVS::AFTER_SBOX_INV[i][byte_j];
            for (size_t bit_i = 0; bit_i < 8; ++bit_i)
            {
                const auto bit = (byte & (1 << bit_i)) ? poly1::set_one() : poly1::set_zero();
                state_p[byte_j][bit_i] = quicksilver_gf2<QSP>(bit, &qs_state_prover);
                state_v[byte_j][bit_i] = quicksilver_gf2<QSV>(bit, &qs_state_verifier);
            }
            gf256_gf2_conjugates<8>(&qs_state_prover, state_with_conjugates_p[byte_j],
                                    state_p[byte_j]);
            gf256_gf2_conjugates<8>(&qs_state_verifier, state_with_conjugates_v[byte_j],
                                    state_v[byte_j]);
        }

        for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
        {
            out_state_with_sq_p[0][byte_j] = sbox_affine<false>(state_with_conjugates_p[byte_j]);
            out_state_with_sq_v[0][byte_j] = sbox_affine<false>(state_with_conjugates_v[byte_j]);
            out_state_with_sq_p[1][byte_j] = sbox_affine<true>(state_with_conjugates_p[byte_j]);
            out_state_with_sq_v[1][byte_j] = sbox_affine<true>(state_with_conjugates_v[byte_j]);
        }

        // compare against expected output state
        for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
        {
            const auto expected_byte = poly_secpar<S>::from_8_byte(AES_TVS::AFTER_SBOX[i][byte_j]);
            const auto expected_byte_sq =
                (expected_byte * expected_byte).template reduce_to<secpar_to_bits(S)>();
            REQUIRE(out_state_with_sq_p[0][byte_j].value() == expected_byte);
            REQUIRE(out_state_with_sq_p[1][byte_j].value() == expected_byte_sq);
            REQUIRE(
                qs_test.check_mac(out_state_with_sq_p[0][byte_j], out_state_with_sq_v[0][byte_j]));
            REQUIRE(
                qs_test.check_mac(out_state_with_sq_p[1][byte_j], out_state_with_sq_v[1][byte_j]));
        }
    }
}

TEMPLATE_TEST_CASE("aes_bitwise_mix_columns_and_add_roundkey_inplace", "[quicksilver]", secpar128_t,
                   secpar192_t, secpar256_t)
{
    constexpr auto S = TestType::value;
    using test_state = quicksilver_test_state<S>;
    using QSP = test_state::QSP;
    using QSV = test_state::QSV;
    constexpr auto O = owf::aes_ecb;
    using OC = OWF_CONSTANTS<S, O>;
    using AES_TVS = aes_tvs<secpar_to_bits(S)>;

    const auto delta = rand<block_secpar<S>>();
    test_state qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    auto state_p = qs_state_prover.template const_gf2_2d_array<8, OC::OWF_BLOCK_SIZE>();
    auto state_v = qs_state_verifier.template const_gf2_2d_array<8, OC::OWF_BLOCK_SIZE>();

    auto rk_p = qs_state_prover.template const_gf2_array<8 * OC::OWF_BLOCK_SIZE>();
    auto rk_v = qs_state_verifier.template const_gf2_array<8 * OC::OWF_BLOCK_SIZE>();

    for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
    {
        const uint8_t byte = static_cast<uint8_t>(byte_j);
        for (size_t bit_i = 0; bit_i < 8; ++bit_i)
        {
            const auto bit = (byte & (1 << bit_i)) ? poly1::set_one() : poly1::set_zero();
            rk_p[8 * byte_j + bit_i] = quicksilver_gf2<QSP>(bit, &qs_state_prover);
            rk_v[8 * byte_j + bit_i] = quicksilver_gf2<QSV>(bit, &qs_state_verifier);
        }
    }

    for (size_t i = 0; i < AES_TVS::n; ++i)
    {
        for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
        {
            const auto byte = AES_TVS::AFTER_SHIFT_ROWS[i][byte_j];
            for (size_t bit_i = 0; bit_i < 8; ++bit_i)
            {
                const auto bit = (byte & (1 << bit_i)) ? poly1::set_one() : poly1::set_zero();
                state_p[byte_j][bit_i] = quicksilver_gf2<QSP>(bit, &qs_state_prover);
                state_v[byte_j][bit_i] = quicksilver_gf2<QSV>(bit, &qs_state_verifier);
            }
        }
        bitwise_mix_columns_and_add_roundkey_inplace<S, O>(&qs_state_prover, state_p, rk_p.data());
        bitwise_mix_columns_and_add_roundkey_inplace<S, O>(&qs_state_verifier, state_v,
                                                           rk_v.data());
        for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
        {
            const auto expected_byte =
                AES_TVS::AFTER_MIX_COLUMNS[i][byte_j] ^ static_cast<uint8_t>(byte_j);
            for (size_t bit_i = 0; bit_i < 8; ++bit_i)
            {
                const auto expected_bit =
                    (expected_byte & (1 << bit_i)) ? poly1::set_one() : poly1::set_zero();
                REQUIRE(state_p[byte_j][bit_i].value() == expected_bit);
                REQUIRE(qs_test.check_mac(state_p[byte_j][bit_i], state_v[byte_j][bit_i]));
            }
        }
    }
}

TEMPLATE_TEST_CASE("aes_bitwise_inverse_shift_rows_and_sbox_affine", "[quicksilver]", secpar128_t,
                   secpar192_t, secpar256_t)
{
    constexpr auto S = TestType::value;
    using test_state = quicksilver_test_state<S>;
    using QSP = test_state::QSP;
    using QSV = test_state::QSV;
    constexpr auto O = owf::aes_ecb;
    using OC = OWF_CONSTANTS<S, O>;
    using AES_TVS = aes_tvs<secpar_to_bits(S)>;

    const auto delta = rand<block_secpar<S>>();
    test_state qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    auto state_p = qs_state_prover.template const_gf2_2d_array<8, OC::OWF_BLOCK_SIZE>();
    auto state_v = qs_state_verifier.template const_gf2_2d_array<8, OC::OWF_BLOCK_SIZE>();
    auto out_state_p = qs_state_prover.template const_gf2_2d_array<8, OC::OWF_BLOCK_SIZE>();
    auto out_state_v = qs_state_verifier.template const_gf2_2d_array<8, OC::OWF_BLOCK_SIZE>();

    // without shift rows
    for (int do_shift_rows = 0; do_shift_rows < 2; ++do_shift_rows)
    {
        for (size_t i = 0; i < AES_TVS::n; ++i)
        {
            // prepare input state
            for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
            {
                const auto byte = do_shift_rows ? AES_TVS::AFTER_SHIFT_ROWS[i][byte_j]
                                                : AES_TVS::AFTER_SBOX[i][byte_j];
                for (size_t bit_i = 0; bit_i < 8; ++bit_i)
                {
                    const auto bit = (byte & (1 << bit_i)) ? poly1::set_one() : poly1::set_zero();
                    state_p[byte_j][bit_i] = quicksilver_gf2<QSP>(bit, &qs_state_prover);
                    state_v[byte_j][bit_i] = quicksilver_gf2<QSV>(bit, &qs_state_verifier);
                }
            }
            if (do_shift_rows)
            {
                bitwise_inverse_shift_rows_and_sbox_affine<S, O, true>(&qs_state_prover,
                                                                       out_state_p, state_p);
                bitwise_inverse_shift_rows_and_sbox_affine<S, O, true>(&qs_state_verifier,
                                                                       out_state_v, state_v);
            }
            else
            {
                bitwise_inverse_shift_rows_and_sbox_affine<S, O, false>(&qs_state_prover,
                                                                        out_state_p, state_p);
                bitwise_inverse_shift_rows_and_sbox_affine<S, O, false>(&qs_state_verifier,
                                                                        out_state_v, state_v);
            }
            // compare against expected output state
            for (size_t byte_j = 0; byte_j < OC::OWF_BLOCK_SIZE; ++byte_j)
            {
                const auto expected_byte = AES_TVS::AFTER_SBOX_INV[i][byte_j];
                for (size_t bit_i = 0; bit_i < 8; ++bit_i)
                {
                    const auto expected_bit =
                        (expected_byte & (1 << bit_i)) ? poly1::set_one() : poly1::set_zero();
                    REQUIRE(out_state_p[byte_j][bit_i].value() == expected_bit);
                    REQUIRE(qs_test.check_mac(out_state_p[byte_j][bit_i], out_state_v[byte_j][bit_i]));
                }
            }
        }
    }
}
