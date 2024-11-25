project('xkcp', 'c')

include_dirs = [
  '.',
  'lib/common/',
  'lib/high/Keccak/',
  'lib/high/Keccak/FIPS202/',
  'lib/low/KeccakP-1600/AVX2/',
  'lib/low/KeccakP-1600/common/',
  'lib/low/KeccakP-1600-times2/SIMD128/', 
  'lib/low/KeccakP-1600-times2/SIMD128/SSSE3-u2/',
  'lib/low/KeccakP-1600-times4/AVX2/',
  'lib/low/KeccakP-1600-times4/AVX2/u12/',
  'lib/low/KeccakP-1600-times8/fallback-on4/', 
]

keccak_sources = [
  'lib/high/Keccak/FIPS202/KeccakHash.c',
  'lib/high/Keccak/KeccakSponge.c',
]
keccak_avx2_sources = [
  'lib/low/KeccakP-1600/AVX2/KeccakP-1600-AVX2.s',
  'lib/low/KeccakP-1600-times2/SIMD128/KeccakP-1600-times2-SIMD128.c',
  'lib/low/KeccakP-1600-times4/AVX2/KeccakP-1600-times4-SIMD256.c',
]

xkcp_lib = static_library('xkcp', keccak_sources, keccak_avx2_sources,
  include_directories : include_dirs)

xkcp_dep = declare_dependency(
  include_directories : include_dirs,
  link_with : xkcp_lib
)
