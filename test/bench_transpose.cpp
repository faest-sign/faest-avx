#include <array>

#include "test.hpp"
#include "transpose.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("bench_transpose_16Nx16", "[.][bench][transpose]", ALL_FAEST_V3_INSTANCES)
{
    using P = TestType;

    constexpr auto N_MULT = P::CONSTS::CRT_NUM_MULT;
    constexpr auto PADDED_N_MULT = ((N_MULT + 15) / 16) * 16;
    std::array<uint16_t, PADDED_N_MULT> input{};
    std::array<uint8_t, 2 * PADDED_N_MULT> output;
    std::generate(input.begin(), input.end(), [] { return rand(); });

    BENCHMARK("transpose_16Nx16")
    {
        transpose_16Nx16(reinterpret_cast<uint8_t*>(output.data()),
                         reinterpret_cast<const uint8_t*>(input.data()), PADDED_N_MULT / 16);
    };
}
