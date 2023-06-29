# Platform Specific FAEST Implementation for x86-64 (With ISA Extensions)

## Versions

The major version number of the FAEST implementation matches the major version number of the FAEST specification it implements.
The implementation is currently at version 1.0.

## Compilation

Building requires GNU make 4.4 or newer.
To build all enabled settings, run `make -j<number_of_threads>`.
Note: it can take a long time to compile them all.
To build just a single setting, run `make -j<number_of_threads> <setting_name>`.
The output will be in `Additional_Implementations/<setting_name>`.

## Settings

There are many different settings of the FAEST implementation, which are all compiled independently from each other.
Each setting is given a name: `sec<security_parameter>_<primitives>_<𝜏>_<platform_setting>`.
Not all settings are included in the FAEST specification, so here are the settings the are:

- sec128_cccs_11: FAEST-128s
- sec128_cccs_16: FAEST-128f
- sec192_cccs_16: FAEST-192s
- sec192_cccs_24: FAEST-192f
- sec256_cccs_22: FAEST-256s
- sec256_cccs_32: FAEST-256f
- sec128_eccs_11: FAEST-EM-128s
- sec128_eccs_16: FAEST-EM-128f
- sec192_eccs_16: FAEST-EM-192s
- sec192_eccs_24: FAEST-EM-192f
- sec256_eccs_22: FAEST-EM-256s
- sec256_eccs_32: FAEST-EM-256f

### Security Parameter

The security parameter can be 128, 192, or 256.

### Primitives

FAEST is built out of hashes, PRGs, and a OWF.
These can be set from the following options:
- c: AES in CTR mode.
- e: AES/Rijndael in Even–Mansour mode. Not available as a PRG for 192-bit security parameter.
- s: SHAKE. Only available for the leaves of the GGM tree.

There are 4 independent primitives that can be set from these options.
1. The one-way function that the signer proves knowledge of a preimage to.
2. The PRG used to expand the seeds used to generate the VOLE correlation.
3. The PRG used to expand the GGM tree.
4. The hash/PRG used to commit to the leaves of the GGM tree.

The letters for these 4 settings are concatenated together in the setting name.

### 𝜏

𝜏 is the number of bits that need to be sent in the signature for each bit of the witness.
Increasing 𝜏 gives faster but longer signatures.

### Platform Settings

We plans for three sets of extensions that the implementation can use:

- AVX2: The AVX2, AES-NI, PCLMULQDQ, and BMI1 instruction set extensions.
- AVX2_VAES: The above, plus VAES and VPCLMULQDQ.
- AVX512: The above, plus AVX512F and AVX512BW.

Currently we have only implemented the AVX2 set.
