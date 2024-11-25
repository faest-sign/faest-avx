project('faest', ['c', 'cpp'],
  version : '0.1.0',
  license : 'MIT',
  default_options : [
    'c_std=c23',
    'cpp_std=c++23',
    'warning_level=3',
    # 'b_sanitize=address',
    'b_ndebug=false',
    'b_lto=true',
    'cpp_args=-O3 -march=native -mtune=native -g',
    'c_args=-O3 -march=native -mtune=native -g',
  ],
)

# Flags from the old Makefile
#   'mtune=native',
# CFLAGS+=-g -O2 -march=native -mtune=native -std=c11
# CPPFLAGS+=-DHAVE_OPENSSL -DNDEBUG -MMD -MP -MF $*.d
# -pedantic-errors
# -Wall -Wexte
# -Wno-ignored-attributes

xkcp = subproject('xkcp').get_variable('xkcp_dep')

randomness_os_dep = declare_dependency(
  link_with : static_library(
    'randomness_os',
    'randomness_os.c',
  ),
  include_directories : '.',
)

faest_include_dirs = include_directories(['.'])
faest_sources = [
  'avx2/aes_impl.cpp',
  'faest.cpp',
  'owf_proof.cpp',
  'polynomials_constants.cpp',
  'sha3/KeccakHashtimes4.c',
  'sha3/KeccakSpongetimes4.c',
  'small_vole.cpp',
  'vector_com.cpp',
  'vole_commit.cpp',
]
faest_lib = static_library('faest',
  faest_sources,
  dependencies : xkcp,
)
faest_dep = declare_dependency(
  link_with : faest_lib,
  include_directories : faest_include_dirs,
  dependencies : xkcp,
)


subdir('test')


pymod = import('python')
python = pymod.find_installation('python3', required: true)

all_names = [
  'faest_128f',
  'faest_128s',
  'faest_192f',
  'faest_192s',
  'faest_256f',
  'faest_256s',
  'faest_em_128f',
  'faest_em_128s',
  'faest_em_192f',
  'faest_em_192s',
  'faest_em_256f',
  'faest_em_256s',
]
run_target(
  'prepare_nist',
  command: [
    python,
    [
      join_paths(meson.project_source_root(), 'tools', 'prepare_nist.py'),
      meson.project_source_root(),
      meson.project_build_root(),
      join_paths(meson.project_build_root(), 'submission'),
    ]
    + all_names,
  ],
)
