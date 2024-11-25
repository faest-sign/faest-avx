#include <array>

#include "test_gfsmall_tvs.hpp"
#include "quicksilver.hpp"
#include "test.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

template <secpar S>
std::array<uint8_t, secpar_to_bytes(S)> BYTES_42 = {
    0x42,
};
template <secpar S> std::array<uint8_t, secpar_to_bytes(S)> BYTES_42INV;
template <>
std::array<uint8_t, secpar_to_bytes(secpar::s128)> BYTES_42INV<secpar::s128> = {
    0x66, 0xe7, 0x9c, 0x73, 0xce, 0x39, 0xe7, 0x9c, 0x73, 0xce, 0x39, 0xe7, 0x9c, 0x73, 0xce, 0xb9,
};
template <>
std::array<uint8_t, secpar_to_bytes(secpar::s192)> BYTES_42INV<secpar::s192> = {
    0xc2, 0xb5, 0xd6, 0x5a, 0x6b, 0xad, 0xb5, 0xd6, 0x5a, 0x6b, 0xad, 0xb5,
    0xd6, 0x5a, 0x6b, 0xad, 0xb5, 0xd6, 0x5a, 0x6b, 0xad, 0xb5, 0xd6, 0xda,
};
template <>
std::array<uint8_t, secpar_to_bytes(secpar::s256)> BYTES_42INV<secpar::s256> = {
    0xa5, 0x59, 0x6b, 0xad, 0xb5, 0xd6, 0x5a, 0x6b, 0xad, 0xb5, 0xd6, 0x5a, 0x6b, 0xad, 0xb5, 0xd6,
    0x5a, 0x6b, 0xad, 0xb5, 0xd6, 0x5a, 0x6b, 0xad, 0xb5, 0xd6, 0x5a, 0x6b, 0xad, 0xb5, 0xd6, 0xda,
};

TEMPLATE_TEST_CASE("one", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    using test_state = quicksilver_test_state<TestType::value>;
    using QSP = test_state::QSP;
    using QSV = test_state::QSV;

    const auto delta = rand<block_secpar<TestType::value>>();
    test_state qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    const auto one_p = quicksilver_gfsecpar<QSP>(1, &qs_state_prover);
    const auto one_v = quicksilver_gfsecpar<QSV>(1, &qs_state_verifier);

    qs_state_prover.add_inverse_constraints(one_p, one_p);
    qs_state_verifier.add_inverse_constraints(one_v, one_v);

    auto [check_prover, check_verifier] = qs_test.compute_check();
    REQUIRE(check_prover == check_verifier);
}

TEMPLATE_TEST_CASE("constant", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    using test_state = quicksilver_test_state<TestType::value>;
    using QSP = test_state::QSP;
    using QSV = test_state::QSV;

    const auto delta = rand<block_secpar<TestType::value>>();
    test_state qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    const auto c_42 = poly_secpar<TestType::value>::load_dup(BYTES_42<TestType::value>.data());
    const auto c_42inv =
        poly_secpar<TestType::value>::load_dup(BYTES_42INV<TestType::value>.data());
    const auto c_42_p = quicksilver_gfsecpar<QSP>(c_42, &qs_state_prover);
    const auto c_42_v = quicksilver_gfsecpar<QSV>(c_42, &qs_state_verifier);
    const auto c_42inv_p = quicksilver_gfsecpar<QSP>(c_42inv, &qs_state_prover);
    const auto c_42inv_v = quicksilver_gfsecpar<QSV>(c_42inv, &qs_state_verifier);

    qs_state_prover.add_inverse_constraints(c_42_p, c_42inv_p);
    qs_state_verifier.add_inverse_constraints(c_42_v, c_42inv_v);

    auto [check_prover, check_verifier] = qs_test.compute_check();
    REQUIRE(check_prover == check_verifier);
}

TEMPLATE_TEST_CASE("mul constant", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    using test_state = quicksilver_test_state<TestType::value>;
    using QSP = test_state::QSP;
    using QSV = test_state::QSV;

    const auto delta = rand<block_secpar<TestType::value>>();
    test_state qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    const auto c_42 = poly_secpar<TestType::value>::load_dup(BYTES_42<TestType::value>.data());
    const auto c_42inv =
        poly_secpar<TestType::value>::load_dup(BYTES_42INV<TestType::value>.data());
    const auto c_42_p = quicksilver_gfsecpar<QSP>(c_42, &qs_state_prover);
    const auto c_42_v = quicksilver_gfsecpar<QSV>(c_42, &qs_state_verifier);
    const auto c_42_x_42inv_p = c_42inv * c_42_p;
    const auto c_42_x_42inv_v = c_42inv * c_42_v;

    qs_state_prover.add_constraint(c_42_x_42inv_p + 1);
    qs_state_verifier.add_constraint(c_42_x_42inv_v + 1);

    auto [check_prover, check_verifier] = qs_test.compute_check();
    REQUIRE(check_prover == check_verifier);
}

TEMPLATE_TEST_CASE("deg1 constraint", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    constexpr auto S = TestType::value;
    using test_state = quicksilver_test_state<S, 1>;

    constexpr auto n = GF256_RAND_XS.size();
    const size_t num_constraints = n;
    std::array<uint8_t, 3 * n + 16> witness;
    const auto witness_bits = 8 * witness.size();
    std::copy_n(GF256_RAND_XS.begin(), n, &witness[0]);
    std::copy_n(GF256_RAND_YS.begin(), n, &witness[n]);
    std::copy_n(GF256_RAND_XY_SUMS.begin(), n, &witness[2 * n]);
    const auto delta = rand<block_secpar<TestType::value>>();
    test_state qs_test(num_constraints, witness.data(), witness_bits, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;
    const auto c_zero = poly_secpar<S>::set_zero();

    for (size_t i = 0; i < n; ++i)
    {
        INFO("i = " << i);
        const auto x_p = qs_state_prover.load_witness_8_bits_and_combine(8 * i);
        const auto x_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * i);
        const auto y_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (n + i));
        const auto y_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (n + i));
        const auto w_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (2 * n + i));
        const auto w_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (2 * n + i));
        const auto c_p = (x_p + y_p) + w_p;
        const auto c_v = (x_v + y_v) + w_v;
        INFO("x_i = 0x" << std::hex << poly_secpar_gf256sub<S>(x_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(x_p.value()) == GF256_RAND_XS[i]);
        INFO("y_i = 0x" << std::hex << poly_secpar_gf256sub<S>(y_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(y_p.value()) == GF256_RAND_YS[i]);
        INFO("x_i + y_i = 0x" << std::hex << poly_secpar_gf256sub<S>(w_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(w_p.value()) == GF256_RAND_XY_SUMS[i]);
        INFO("c_i = 0x" << std::hex << poly_secpar_gf256sub<S>(c_p.value()));
        REQUIRE(qs_test.check_mac(c_p, c_v));
        REQUIRE(poly_secpar_gf256sub<S>(c_p.value()) == 0x00);
        REQUIRE(c_p.value() == c_zero);
        qs_state_prover.add_constraint(c_p);
        qs_state_verifier.add_constraint(c_v);
    }

    auto [check_prover, check_verifier] = qs_test.compute_check();
    REQUIRE(check_prover == check_verifier);
}

TEMPLATE_TEST_CASE("deg2 constraint", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    constexpr auto S = TestType::value;
    using test_state = quicksilver_test_state<S, 2>;

    constexpr auto n = GF256_RAND_XS.size();
    const size_t num_constraints = n;
    std::array<uint8_t, 3 * n + 16> witness;
    const auto witness_bits = 8 * witness.size();
    std::copy_n(GF256_RAND_XS.begin(), n, &witness[0]);
    std::copy_n(GF256_RAND_YS.begin(), n, &witness[n]);
    std::copy_n(GF256_RAND_XY_PRODUCTS.begin(), n, &witness[2 * n]);
    const auto delta = rand<block_secpar<TestType::value>>();
    test_state qs_test(num_constraints, witness.data(), witness_bits, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;
    const auto c_zero = poly_secpar<S>::set_zero();

    for (size_t i = 0; i < n; ++i)
    {
        INFO("i = " << i);
        const auto x_p = qs_state_prover.load_witness_8_bits_and_combine(8 * i);
        const auto x_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * i);
        const auto y_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (n + i));
        const auto y_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (n + i));
        const auto w_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (2 * n + i));
        const auto w_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (2 * n + i));
        const auto c_p = (x_p * y_p) + w_p;
        const auto c_v = (x_v * y_v) + w_v;
        INFO("x_i = 0x" << std::hex << poly_secpar_gf256sub<S>(x_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(x_p.value()) == GF256_RAND_XS[i]);
        INFO("y_i = 0x" << std::hex << poly_secpar_gf256sub<S>(y_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(y_p.value()) == GF256_RAND_YS[i]);
        INFO("x_i * y_i = 0x" << std::hex << poly_secpar_gf256sub<S>(w_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(w_p.value()) == GF256_RAND_XY_PRODUCTS[i]);
        INFO("c_i = 0x" << std::hex << poly_secpar_gf256sub<S>(c_p.value()));
        REQUIRE(qs_test.check_mac(c_p, c_v));
        REQUIRE(poly_secpar_gf256sub<S>(c_p.value()) == 0x00);
        REQUIRE(c_p.value() == c_zero);
        qs_state_prover.add_constraint(c_p);
        qs_state_verifier.add_constraint(c_v);
    }

    auto [check_prover, check_verifier] = qs_test.compute_check();
    REQUIRE(check_prover == check_verifier);
}

TEMPLATE_TEST_CASE("deg3 constraint", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    constexpr auto S = TestType::value;
    using test_state = quicksilver_test_state<S, 3>;

    constexpr auto n = GF256_RAND_XS.size();
    const size_t num_constraints = n;
    std::array<uint8_t, 4 * n + 16> witness;
    const auto witness_bits = 8 * witness.size();
    std::copy_n(GF256_RAND_XS.begin(), n, &witness[0]);
    std::copy_n(GF256_RAND_YS.begin(), n, &witness[n]);
    std::copy_n(GF256_RAND_ZS.begin(), n, &witness[2 * n]);
    std::copy_n(GF256_RAND_XYZ_PRODUCTS.begin(), n, &witness[3 * n]);
    const auto delta = rand<block_secpar<TestType::value>>();
    test_state qs_test(num_constraints, witness.data(), witness_bits, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;
    const auto c_zero = poly_secpar<S>::set_zero();

    for (size_t i = 0; i < n; ++i)
    {
        INFO("i = " << i);
        const auto x_p = qs_state_prover.load_witness_8_bits_and_combine(8 * i);
        const auto x_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * i);
        const auto y_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (n + i));
        const auto y_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (n + i));
        const auto z_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (2 * n + i));
        const auto z_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (2 * n + i));
        const auto w_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (3 * n + i));
        const auto w_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (3 * n + i));
        const auto c_p = (x_p * y_p * z_p) + w_p;
        const auto c_v = (x_v * y_v * z_v) + w_v;
        INFO("x_i = 0x" << std::hex << poly_secpar_gf256sub<S>(x_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(x_p.value()) == GF256_RAND_XS[i]);
        INFO("y_i = 0x" << std::hex << poly_secpar_gf256sub<S>(y_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(y_p.value()) == GF256_RAND_YS[i]);
        INFO("z_i = 0x" << std::hex << poly_secpar_gf256sub<S>(z_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(z_p.value()) == GF256_RAND_ZS[i]);
        INFO("x_i * y_i * z_i = 0x" << std::hex << poly_secpar_gf256sub<S>(w_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(w_p.value()) == GF256_RAND_XYZ_PRODUCTS[i]);
        INFO("c_i = 0x" << std::hex << poly_secpar_gf256sub<S>(c_p.value()));
        REQUIRE(qs_test.check_mac(c_p, c_v));
        REQUIRE(poly_secpar_gf256sub<S>(c_p.value()) == 0x00);
        REQUIRE(c_p.value() == c_zero);
        qs_state_prover.add_constraint(c_p);
        qs_state_verifier.add_constraint(c_v);
    }

    auto [check_prover, check_verifier] = qs_test.compute_check();
    REQUIRE(check_prover == check_verifier);
}

TEMPLATE_TEST_CASE("deg2-deg3 constraint", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    constexpr auto S = TestType::value;
    using test_state = quicksilver_test_state<S, 3>;

    constexpr auto n = GF256_RAND_XS.size();
    const size_t num_constraints = 2 * n;
    std::array<uint8_t, 5 * n + 16> witness;
    const auto witness_bits = 8 * witness.size();
    std::copy_n(GF256_RAND_XS.begin(), n, &witness[0]);
    std::copy_n(GF256_RAND_YS.begin(), n, &witness[n]);
    std::copy_n(GF256_RAND_ZS.begin(), n, &witness[2 * n]);
    std::copy_n(GF256_RAND_XY_PRODUCTS.begin(), n, &witness[3 * n]);
    std::copy_n(GF256_RAND_XYZ_PRODUCTS.begin(), n, &witness[4 * n]);
    const auto delta = rand<block_secpar<TestType::value>>();
    test_state qs_test(num_constraints, witness.data(), witness_bits, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;
    const auto c_zero = poly_secpar<S>::set_zero();

    for (size_t i = 0; i < n; ++i)
    {
        INFO("i = " << i);
        const auto x_p = qs_state_prover.load_witness_8_bits_and_combine(8 * i);
        const auto x_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * i);
        const auto y_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (n + i));
        const auto y_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (n + i));
        const auto z_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (2 * n + i));
        const auto z_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (2 * n + i));

        INFO("x_i = 0x" << std::hex << poly_secpar_gf256sub<S>(x_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(x_p.value()) == GF256_RAND_XS[i]);
        INFO("y_i = 0x" << std::hex << poly_secpar_gf256sub<S>(y_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(y_p.value()) == GF256_RAND_YS[i]);
        INFO("z_i = 0x" << std::hex << poly_secpar_gf256sub<S>(z_p.value()));
        REQUIRE(poly_secpar_gf256sub<S>(z_p.value()) == GF256_RAND_ZS[i]);

        {
            const auto w_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (3 * n + i));
            const auto w_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (3 * n + i));
            const auto c_p = (x_p * y_p) + w_p;
            const auto c_v = (x_v * y_v) + w_v;
            INFO("x_i * y_i * z_i = 0x" << std::hex << poly_secpar_gf256sub<S>(w_p.value()));
            REQUIRE(poly_secpar_gf256sub<S>(w_p.value()) == GF256_RAND_XY_PRODUCTS[i]);
            INFO("c_i = 0x" << std::hex << poly_secpar_gf256sub<S>(c_p.value()));
            REQUIRE(qs_test.check_mac(c_p, c_v));
            REQUIRE(poly_secpar_gf256sub<S>(c_p.value()) == 0x00);
            REQUIRE(c_p.value() == c_zero);
            qs_state_prover.add_constraint(c_p);
            qs_state_verifier.add_constraint(c_v);
        }

        {
            const auto w_p = qs_state_prover.load_witness_8_bits_and_combine(8 * (4 * n + i));
            const auto w_v = qs_state_verifier.load_witness_8_bits_and_combine(8 * (4 * n + i));
            const auto c_p = (x_p * y_p * z_p) + w_p;
            const auto c_v = (x_v * y_v * z_v) + w_v;
            INFO("x_i * y_i * z_i = 0x" << std::hex << poly_secpar_gf256sub<S>(w_p.value()));
            REQUIRE(poly_secpar_gf256sub<S>(w_p.value()) == GF256_RAND_XYZ_PRODUCTS[i]);
            INFO("c_i = 0x" << std::hex << poly_secpar_gf256sub<S>(c_p.value()));
            REQUIRE(qs_test.check_mac(c_p, c_v));
            REQUIRE(poly_secpar_gf256sub<S>(c_p.value()) == 0x00);
            REQUIRE(c_p.value() == c_zero);
            qs_state_prover.add_constraint(c_p);
            qs_state_verifier.add_constraint(c_v);
        }
    }

    auto [check_prover, check_verifier] = qs_test.compute_check();
    REQUIRE(check_prover == check_verifier);
}

/*
TEMPLATE_TEST_CASE("inverse", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    const auto delta = rand<block_secpar<TestType::value>>();
    quicksilver_test_state<TestType::value> qs_test(1, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    const auto c_42 = poly_secpar_vec<TestType::value>::load_dup(BYTES_42<TestType::value>.data());
    const auto c_42inv =
        poly_secpar_vec<TestType::value>::load_dup(BYTES_42INV<TestType::value>.data());
    const auto c_42_p = qs_state_prover.const_gfsecpar(c_42);
    const auto c_42_v = qs_state_verifier.const_gfsecpar(c_42);
    const auto c_42inv_p = qs_state_prover.const_gfsecpar(c_42inv);
    const auto c_42inv_v = qs_state_verifier.const_gfsecpar(c_42inv);

    qs_state_prover.add_inverse_constraints(c_42_p, c_42inv_p);
    qs_state_verifier.add_inverse_constraints(c_42_v, c_42inv_v);

    auto [check_prover, check_verifier] = qs_test.compute_check();
    REQUIRE(check_prover == check_verifier);
}

TEMPLATE_TEST_CASE("inverse_or_zero", "[quicksilver]", secpar128_t, secpar192_t, secpar256_t)
{
    constexpr auto secpar_bits = secpar_to_bits(TestType::value);
    const auto delta = rand<block_secpar<TestType::value>>();
    quicksilver_test_state<TestType::value> qs_test(2, NULL, 0, delta);
    auto& qs_state_prover = qs_test.prover_state;
    auto& qs_state_verifier = qs_test.verifier_state;

    const auto c_42 = poly_secpar_vec<TestType::value>::load_dup(BYTES_42<TestType::value>.data());
    const auto c_42sq = (c_42 * c_42).template reduce_to<secpar_bits>();
    const auto c_42inv =
        poly_secpar_vec<TestType::value>::load_dup(BYTES_42INV<TestType::value>.data());
    const auto c_42invsq = (c_42inv * c_42inv).template reduce_to<secpar_bits>();
    const auto c_42_p = qs_state_prover.const_gfsecpar(c_42);
    const auto c_42_v = qs_state_verifier.const_gfsecpar(c_42);
    const auto c_42sq_p = qs_state_prover.const_gfsecpar(c_42sq);
    const auto c_42sq_v = qs_state_verifier.const_gfsecpar(c_42sq);
    const auto c_42inv_p = qs_state_prover.const_gfsecpar(c_42inv);
    const auto c_42inv_v = qs_state_verifier.const_gfsecpar(c_42inv);
    const auto c_42invsq_p = qs_state_prover.const_gfsecpar(c_42invsq);
    const auto c_42invsq_v = qs_state_verifier.const_gfsecpar(c_42invsq);
    const auto zero_p = qs_state_prover.zero_gfsecpar();
    const auto zero_v = qs_state_verifier.zero_gfsecpar();

    qs_state_prover.add_inverse_or_zero_constraints(c_42_p, c_42sq_p, c_42inv_p, c_42invsq_p);
    qs_state_verifier.add_inverse_or_zero_constraints(c_42_v, c_42sq_v, c_42inv_v, c_42invsq_v);
    qs_state_prover.add_inverse_or_zero_constraints(zero_p, zero_p, zero_p, zero_p);
    qs_state_verifier.add_inverse_or_zero_constraints(zero_v, zero_v, zero_v, zero_v);

    auto [check_prover, check_verifier] = qs_test.compute_check();
    REQUIRE(check_prover == check_verifier);
}
*/
