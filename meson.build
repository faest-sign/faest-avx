project('faest', ['c', 'cpp'],
  version : '0.1.0',
  license : 'MIT',
  default_options : [
    'c_std=c23',
    'cpp_std=c++23',
    'warning_level=3',
    'b_ndebug=if-release',
    'cpp_args=-g -Wno-psabi',
    'c_args=-g',
  ],
)

cpu_family = host_machine.cpu_family()
if cpu_family == 'aarch64'
  arch = 'aarch64'
  add_global_arguments('-march=armv8.2-a+crypto', '-mtune=native', language : ['c', 'cpp'])
elif cpu_family == 'x86_64'
  arch = 'avx2'
  add_global_arguments('-march=native', '-mtune=native', language : ['c', 'cpp'])
else
  error('Unsupported CPU family: ' + cpu_family)
endif

# Flags from the old Makefile
#   'mtune=native',
# CFLAGS+=-g -O2 -march=native -mtune=native -std=c11
# CPPFLAGS+=-DHAVE_OPENSSL -DNDEBUG -MMD -MP -MF $*.d
# -pedantic-errors
# -Wall -Wexte
# -Wno-ignored-attributes

xkcp = subproject('xkcp').get_variable('xkcp_dep')

cc = meson.get_compiler('c')
randomness_args = []
if cc.has_header_symbol('sys/random.h', 'getrandom')
  randomness_args += ['-DHAVE_SYS_RANDOM_H', '-DHAVE_GETRANDOM']
elif cc.has_function('arc4random_buf')
  # On macOS/BSD this avoids open()+read()+close() on /dev/urandom per call,
  # which costs ~45us and dominates key generation.
  randomness_args += ['-DHAVE_ARC4RANDOM_BUF']
endif

randomness_os_dep = declare_dependency(
  link_with : static_library(
    'randomness_os',
    'randomness_os.c',
    c_args : randomness_args,
  ),
  include_directories : '.',
)


# CRT Table Generation
py3 = import('python').find_installation('python3')
crt_presets = ['128f', '128s', '192f', '192f_em', '192s', '192s_em', '256f', '256s']
crt_jsons = []
crt_includes = ''
foreach j : range(crt_presets.length())
  json_name = 'crt_constants_' + crt_presets[j] + '.json'
  crt_jsons += custom_target(
    json_name,
    output : json_name,
    command : [
      py3, files('tools/vole_mult_tables.py'),
      '--preset', crt_presets[j], '--out', '@OUTPUT@',
    ],
  )
endforeach
crt_cpps = []
crt_headers = []
foreach j : range(crt_jsons.length())
  cpp_name = 'crt_constants_' + crt_presets[j] + '.cpp'
  hpp_name = 'crt_constants_' + crt_presets[j] + '.hpp'
  crt_includes += '#include "' + hpp_name + '"\n'
  crt_cpps += custom_target(
    cpp_name,
    input : crt_jsons[j],
    output : cpp_name,
    command : [py3, files('tools/gen_crt_tables.py'), '@INPUT@'],
    capture : true,
  )
  crt_headers += custom_target(
    hpp_name,
    input : crt_jsons[j],
    output : hpp_name,
    command : [py3, files('tools/gen_crt_tables.py'), '--header', '@INPUT@'],
    capture : true,
  )
endforeach
crt_headers += custom_target(
  'generated_crt_constants.hpp',
  output : 'generated_crt_constants.hpp',
  command : [py3, '-c', 'open("' + meson.current_build_dir() + '/generated_crt_constants.hpp' + '", "w").write("""' + crt_includes + '""")'],
)

faest_include_dirs = include_directories(['.'])
faest_sources = [
  'aes.cpp',
  'faest.cpp',
  'owf_proof_key_sched.cpp',
  'owf_proof_enc_v2.cpp',
  'owf_proof_enc_v3.cpp',
  'owf_proof_v2.cpp',
  'owf_proof_v2_em.cpp',
  'owf_proof_v3.cpp',
  'owf_proof_v3_em.cpp',
  'polynomials_constants.cpp',
  'sha3/KeccakHash-times4.c',
  'sha3/KeccakSponge-times4.c',
  'small_vole.cpp',
  'vector_com.cpp',
  'vole_commit.cpp',
] + crt_cpps + crt_headers
faest_lib = static_library('faest',
  faest_sources,
  dependencies : xkcp,
)
faest_dep = declare_dependency(
  link_with : faest_lib,
  include_directories : faest_include_dirs,
  dependencies : xkcp,
  sources : crt_headers,
)

print_parameters = executable('print_parameters',
  ['print_parameters.cpp'] + crt_headers,
  include_directories : faest_include_dirs,
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
      arch,
    ]
    + all_names,
  ],
)
