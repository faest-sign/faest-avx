catch2 = subproject('catch2')
catch2_with_main_dep = catch2.get_variable('catch2_with_main_dep')

test_sources = [
  'test_aes.cpp',
  'test_aes_tvs.cpp',
  'test_bavc_tvs.cpp',
  'test_faest.cpp',
  'test_faest_experimental.cpp',
  'test_faest_tvs.cpp',
  'test_faest_v2_tvs.cpp',
  'test_gfsmall.cpp',
  'test_gfsmall_tvs.cpp',
  'test_parameters.cpp',
  'test_polynomials.cpp',
  'test_quicksilver.cpp',
  'test_owf_proof.cpp',
  'test_owf_proof_tools.cpp',
  'test_prg.cpp',
  'test_prg_tvs.cpp',
  'test_small_vole.cpp',
  'test_universal_hash.cpp',
  'test_transpose.cpp',
  'test_vector_com.cpp',
  'test_vole_commit.cpp',
  'test_vole_commit_tvs.cpp',
  'test_vole_commit_tvs_v2.cpp',
  'test_witness.cpp',
]
executable('tests',
  test_sources,
  dependencies : [catch2_with_main_dep, faest_dep]
)
executable('bench',
  'bench_api.cpp',
  'bench_components.cpp',
  'bench_leaf_hash.cpp',
  'bench_variants.cpp',
  dependencies : [catch2_with_main_dep, faest_dep, randomness_os_dep]
)
