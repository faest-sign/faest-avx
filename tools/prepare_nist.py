# SPDX-License-Identifier: MIT

import sys
import shutil
import subprocess
import os
from pathlib import Path

sig_sizes = {
    'faest_128s': 4066,
    'faest_128f': 5170,
    'faest_em_128s': 3466,
    'faest_em_128f': 4170,
    'faest_192s': 9410,
    'faest_192f': 11738,
    'faest_em_192s': 7874,
    'faest_em_192f': 9818,
    'faest_256s': 16626,
    'faest_256f': 20856,
    'faest_em_256s': 14554,
    'faest_em_256f': 18084,
}

aarch64_cflags = '-march=armv8.2-a+crypto'
aarch64_cxxflags = '-march=armv8.2-a+crypto'


def generate_crt_tables(
    project_root: Path, outdir: Path
) -> None:
    print('Generate CRT Tables')
    subprocess.check_call(['python3', project_root / 'tools' / 'vole_mult_tables.py', '--all', '--outdir', outdir], cwd=outdir)


def generate(
    project_root: Path, build_root: Path, target_root: Path, param_name: str, arch: str, crt_tables: Path
) -> None:
    target = (target_root / "Additional_Implementations" / arch / param_name).absolute()
    target_kat = (target_root / "KAT" / param_name).absolute()
    print(
        f"Preparing {param_name}: root: {project_root}, build root: {build_root}, target: {target}"
    )
    target.mkdir(parents=True, exist_ok=True)
    (target / arch).mkdir(parents=True, exist_ok=True)
    (target / "common").mkdir(parents=True, exist_ok=True)
    target_kat.mkdir(parents=True, exist_ok=True)

    target_sha3 = target / "sha3"
    target_sha3.mkdir(parents=True, exist_ok=True)
    target_nist_kat = target / "NIST-KATs"
    target_nist_kat.mkdir(parents=True, exist_ok=True)
    target_tables = target / "tables"
    target_tables.mkdir(parents=True, exist_ok=True)
    target_tests = target / "tests"
    target_tests.mkdir(parents=True, exist_ok=True)

    sha3_sources = project_root / "sha3"
    xkcp_sources = project_root / "subprojects/xkcp"
    test_sources = project_root / "tests"
    tools_sources = project_root / "tools"

    # copy FAEST implementation
    for glob in ["*.c", "*.cpp", "*.h", "*.hpp", "*.inc"]:
        for source in project_root.glob(glob):
            if source.suffix == '.cpp' and ('_v1' in source.name or '_v2' in source.name):
                continue
            shutil.copy(source, target)
    for glob in ["common/*.cpp", "common/*.hpp", "common/*.inc"]:
        for source in project_root.glob(glob):
            shutil.copy(source, target / "common")
    for glob in [f"{arch}/*.cpp", f"{arch}/*.hpp", f"{arch}/*.inc"]:
        for source in project_root.glob(glob):
            shutil.copy(source, target / arch)

    # find parameters.
    secpar = int(param_name[-4:-1])
    sk_size = (2 * secpar // 8) if 'em' in param_name else (16 + secpar // 8)
    pk_size = (2 * secpar // 8) if 'em' in param_name else (16 + 16 * ((secpar + 127) // 128))
    sig_size = sig_sizes[param_name]
    params_type = 'v3::' + param_name[:-1] + '_' + param_name[-1:]

    # generate files
    for source in ['api.h', 'api.cpp']:
        subprocess.call(['sed',
            '-e', f's/%SECRETKEYBYTES%/{sk_size}/g',
            '-e', f's/%PUBLICKEYBYTES%/{pk_size}/g',
            '-e', f's/%SIGBYTES%/{sig_size}/g',
            '-e', f's/%VERSION%/{param_name}/g',
            '-e', f's/%PARAMSTYPE%/{params_type}/g',
            project_root / (source + '.in')], stdout=open(target / source, 'w'))

    # copy sha3 sources
    for source in sha3_sources.glob("*.c"):
        shutil.copy(source, target_sha3)
    for header in sha3_sources.glob("*.h"):
        shutil.copy(header, target_sha3)
    for source in sha3_sources.glob("*.macros"):
        shutil.copy(source, target_sha3)
    for source in sha3_sources.glob("*.inc"):
        shutil.copy(source, target_sha3)
    sha3_sources = sha3_sources / "opt64"
    for source in sha3_sources.glob("*.c"):
        shutil.copy(source, target_sha3)
    for header in sha3_sources.glob("*.h"):
        shutil.copy(header, target_sha3)
    for source in sha3_sources.glob("*.macros"):
        shutil.copy(source, target_sha3)
    for source in sha3_sources.glob("*.inc"):
        shutil.copy(source, target_sha3)

    # copy XKCP sources
    xkcp_globs = [
        'config.h',
        'lib/common/*.h',
        'lib/high/Keccak/*',
        'lib/high/Keccak/FIPS202/*',
        'lib/low/common/*.h',
        'lib/low/common/*.inc',
        'lib/low/KeccakP-1600/common/*',
        'lib/high/Keccak/FIPS202/KeccakHash.c',
        'lib/high/Keccak/KeccakSponge.c',
    ]
    if arch == 'avx2':
        xkcp_globs += [
          'lib/low/KeccakP-1600/AVX2/*',
          'lib/low/KeccakP-1600/AVX2/SnP/*',
          'lib/low/KeccakP-1600-times4/AVX2/*',
          'lib/low/KeccakP-1600-times4/AVX2/SnP/*',
          'lib/low/KeccakP-1600/AVX2/KeccakP-1600-AVX2.s',
          'lib/low/KeccakP-1600-times4/AVX2/KeccakP-1600-times4-AVX2.c',
        ]
    else:
        xkcp_globs += [
          'lib/low/KeccakP-1600/plain-64bits/*',
          'lib/low/KeccakP-1600/plain-64bits/SnP/*',
          'lib/low/KeccakP-1600-times4/fallback-on1/*',
          'lib/low/KeccakP-1600/plain-64bits/KeccakP-1600-opt64.c',
          'lib/low/KeccakP-1600-times4/fallback-on1/KeccakP-1600-times4-on1.c',
        ]
    for glob in xkcp_globs:
        for source in xkcp_sources.glob(glob):
            if source.is_file():
                shutil.copy(source, target_sha3)

    # copy tests
    for test_source in ("api_test.c",):
        shutil.copy(tools_sources / test_source, target_tests)
    # copy NIST files
    for tool_source in ("rng.c", "rng.h", "PQCgenKAT_sign.c"):
        shutil.copy(tools_sources / tool_source, target_nist_kat)
    for tool_source in ("Makefile",):
        shutil.copy(tools_sources / tool_source, target)

    print(f"Generating CRT tables")
    # assume param_name of the form 'faest(_em)?_(128|192|256)(f|s)',
    all_presets = ['128f', '128s', '192f', '192f_em', '192s', '192s_em', '256f', '256s']
    if param_name == 'faest_em_192f':
        table_preset = '192f_em'
    elif param_name == 'faest_em_192s':
        table_preset = '192s_em'
    else:
        table_preset = param_name[-4:]

    with open(target / f'generated_crt_constants.hpp', 'w') as f:
        for preset in all_presets:
            f.write(f'#include "crt_constants_{preset}.hpp"\n')

    for preset in all_presets:
        with open(target / f'crt_constants_{preset}.hpp', 'w') as f:
            subprocess.check_call(['python3', tools_sources / "gen_crt_tables.py", '--header', crt_tables / f'tables_{preset}.json'], cwd=target, stdout=f)
    with open(target / f'crt_constants_{table_preset}.cpp', 'w') as f:
        subprocess.check_call(['python3', tools_sources / "gen_crt_tables.py", crt_tables / f'tables_{table_preset}.json'], cwd=target, stdout=f)

    # build and create KATs
    print(f"Building {param_name}")
    cpu_count = os.cpu_count()
    subprocess.check_call(
        ["make"] if cpu_count is None else ["make", "-j", str(max(2, cpu_count - 1))],
        cwd=target,
        env=os.environ.copy() | {'CFLAGS': aarch64_cflags, 'CXXFLAGS': aarch64_cxxflags} if arch == 'aarch64' else None,
    )

    print(f"running api_test")
    subprocess.check_call(target_tests / "api_test", cwd=target_kat)

    print(f"Generating KATs for {param_name}")
    subprocess.check_call(target_nist_kat / "PQCgenKAT_sign", cwd=target_kat)

    subprocess.check_call(["make", "clean"], cwd=target)


def usage_and_exit():
    print(f'Usage: {sys.argv[0]} <project_root> <build_root> <target_root> [avx2|aarch64] param1 [param2 ...]')
    exit(1)

def main():
    if len(sys.argv) < 6:
        usage_and_exit()
    project_root = Path(sys.argv[1])
    build_root = Path(sys.argv[2])
    target_root = Path(sys.argv[3])
    arch = sys.argv[4]
    if arch not in ['avx2', 'aarch64']:
        usage()
    param_names = sys.argv[5:]
    crt_tables = build_root / 'crt_tables'
    crt_tables.mkdir(parents=True, exist_ok=True)

    generate_crt_tables(project_root, crt_tables)

    for param_name in param_names:
        generate(project_root, build_root, target_root, param_name, arch, crt_tables)


if __name__ == "__main__":
    main()
