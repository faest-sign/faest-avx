# Platform Specific FAEST Implementation for x86-64 (With ISA Extensions)

## Versions

This is the C++ implementation of FAEST.
It is based on the previous C implementation, which you can find
[here](https://github.com/faest-sign/faest-avx/tree/release).

The major version number of the FAEST implementation matches the major version number of the FAEST
specification it implements.
The implementation is currently at version 2.0.1.

## Compilation

Building requires (tested versions in parentheses):
- GCC (>= 14.2.0) or Clang (>= 19.1.7)
- [Meson](https://mesonbuild.com/) (>= 1.7.0)
- [Ninja](https://ninja-build.org/) (>= 1.12.1)

The dependencies [XKCP](https://github.com/XKCP/XKCP) for SHAKE and
[Catch2](https://github.com/catchorg/Catch2) for tests/benchmarks are automatically downloaded
during the build process.

### Development

To compile a debug build:
```shell
meson setup build_debug --buildtype=debug
# alternatively use `--buildtype=debugoptimized` to compile with optimizations
# optionally add `-Db_sanitize=address` to enable AddressSanitizer
cd build_debug
meson compile
./test/tests  # to run tests
```

### Benchmarking

To compile a version suitable for benchmarking:
```shell
meson setup build_bench --buildtype=release -Db_lto=true -Doptimization=3
cd build_bench
meson compile
./test/bench '[bench]'  # to run benchmarks
```

### Submission Package and KATs

To create the submission package with separate directories for all FAEST version and to generate the
KATs:
```shell
# create and enter build directory as above
meson compile prepare_nist
```
NB: This can take a while since it compiles FAEST twelve times.


## Settings

There are many different settings of the FAEST implementation.
A parameter set is defined by instantiating the template `parameter_set` in
[`parameters.hpp`](parameters.hpp).
This defines compile-time constants for all the switches and sizes used throughout the
implementation.
By default, we include parameter sets that correspond to the FAEST v1[^1] and the FAEST v2 variants.

[^1]: Note that the v1 variant are not bit-compatible to the FAEST v1 specification, since we changed
some details of the signature scheme.

A FAEST instance is defined by the following parameters:

- The security parameter λ (128, 192, or 256).
- The number of bits 𝜏 that need to be sent in the signature for each bit of the witness.
  Increasing 𝜏 gives faster but longer signatures.
- The one-way function that the signer proves knowledge of a preimage to (ECB or Even-Mansour with
  different optimizations).
- The PRG used to expand the seeds used to generate the VOLE correlation (AES-CTR or fixed-key
  Rijndael).
- The PRG used to expand the nodes in the GGM trees (AES-CTR or fixed-key Rijndael).
- The hash/PRG used to commit to the leaves of the GGM tree (PRG-based hashes or SHAKE).
- The number of zero bits in Delta w_grind.
- The Batch All-But-One Vector Commitment (BAVC) (forest of GGM trees, or the [One-Tree
  BAVC](https://eprint.iacr.org/2024/490) with a opening size threshold T_open).

For more details, we refer to the [FAEST v2 specification](https://faest.info/resources.html).


### Platform Settings

Currently our implementation is using the following set of instruction set extensions:

- AVX2: The AVX2, AES-NI, PCLMULQDQ, and BMI1 instruction set extensions.

Additionally, we plan to support also the following sets of extensions that the implementation can
use:
- AVX2_VAES: The above, plus VAES and VPCLMULQDQ.
- AVX512: The above, plus AVX512F and AVX512BW.
