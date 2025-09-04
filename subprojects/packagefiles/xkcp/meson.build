project('xkcp', 'c')

keccak_include_dirs = [
  '.',
  'lib/common/',
  'lib/high/Keccak/',
  'lib/high/Keccak/FIPS202/',
  'lib/low/common/',
  'lib/low/KeccakP-1600/common/',
]

keccak_sources = [
  'lib/high/Keccak/FIPS202/KeccakHash.c',
  'lib/high/Keccak/KeccakSponge.c',
]
keccak_generic_include_dirs = [
  'lib/low/KeccakP-1600/plain-64bits/',
  'lib/low/KeccakP-1600/plain-64bits/SnP/',
  'lib/low/KeccakP-1600-times4/fallback-on1/',
]
keccak_generic_sources = [
  'lib/low/KeccakP-1600/plain-64bits/KeccakP-1600-opt64.c',
  'lib/low/KeccakP-1600-times4/fallback-on1/KeccakP-1600-times4-on1.c',
]
keccak_avx2_include_dirs = [
  'lib/low/KeccakP-1600/AVX2/',
  'lib/low/KeccakP-1600/AVX2/SnP/',
  'lib/low/KeccakP-1600-times4/AVX2/',
  'lib/low/KeccakP-1600-times4/AVX2/SnP/',
]
keccak_avx2_sources = [
  'lib/low/KeccakP-1600/AVX2/KeccakP-1600-AVX2.s',
  'lib/low/KeccakP-1600-times4/AVX2/KeccakP-1600-times4-AVX2.c',
]

if host_machine.cpu_family() == 'x86_64'
  keccak_include_dirs += keccak_avx2_include_dirs
  keccak_sources += keccak_avx2_sources
else
  keccak_include_dirs += keccak_generic_include_dirs
  keccak_sources += keccak_generic_sources
endif

xkcp_lib = static_library('xkcp', keccak_sources,
  include_directories : keccak_include_dirs)

xkcp_dep = declare_dependency(
  include_directories : keccak_include_dirs,
  link_with : xkcp_lib
)
