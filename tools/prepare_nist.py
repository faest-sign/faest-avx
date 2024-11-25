# SPDX-License-Identifier: MIT

import sys
import shutil
import subprocess
import os
from pathlib import Path

sig_sizes = {
    'faest_128s': 4506,
    'faest_128f': 5924,
    'faest_em_128s': 3906,
    'faest_em_128f': 5060,
    'faest_192s': 11260,
    'faest_192f': 14948,
    'faest_em_192s': 9340,
    'faest_em_192f': 12380,
    'faest_256s': 20696,
    'faest_256f': 26548,
    'faest_em_256s': 17984,
    'faest_em_256f': 23476
}

def generate(
    project_root: Path, build_root: Path, target_root: Path, param_name: str
) -> None:
    target = (target_root / "Additional_Implementations/avx2" / param_name).absolute()
    target_kat = (target_root / "KAT" / param_name).absolute()
    print(
        f"Preparing {param_name}: root: {project_root}, build root: {build_root}, target: {target}"
    )
    target.mkdir(parents=True, exist_ok=True)
    (target / "avx2").mkdir(parents=True, exist_ok=True)
    target_kat.mkdir(parents=True, exist_ok=True)

    target_sha3 = target / "sha3"
    target_sha3.mkdir(parents=True, exist_ok=True)
    target_nist_kat = target / "NIST-KATs"
    target_nist_kat.mkdir(parents=True, exist_ok=True)
    target_tests = target / "tests"
    target_tests.mkdir(parents=True, exist_ok=True)

    sha3_sources = project_root / "sha3"
    xkcp_sources = project_root / "subprojects/xkcp"
    test_sources = project_root / "tests"
    tools_sources = project_root / "tools"

    # copy FAEST implementation
    for glob in ["*.c", "*.cpp", "*.h", "*.hpp", "*.inc"]:
        for source in project_root.glob(glob):
            shutil.copy(source, target)
    for glob in ["avx2/*.cpp", "avx2/*.hpp", "avx2/*.inc"]:
        for source in project_root.glob(glob):
            shutil.copy(source, target / "avx2")

    # find parameters.
    secpar = int(param_name[-4:-1])
    sk_size = (2 * secpar // 8) if 'em' in param_name else (16 + secpar // 8)
    pk_size = (2 * secpar // 8) if 'em' in param_name else (16 + 16 * ((secpar + 127) // 128))
    sig_size = sig_sizes[param_name]
    params_type = 'v2::' + param_name[:-1] + '_' + param_name[-1:]

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
    for glob in [
        'config.h',
        'lib/high/Keccak/FIPS202/KeccakHash.*',
        'lib/high/Keccak/KeccakSponge.*',
        'lib/common/*',
        'lib/low/common/*',
        'lib/low/KeccakP-1600/common/*',
        'lib/low/KeccakP-1600/AVX2/*',
        'lib/low/KeccakP-1600-times2/SIMD128/KeccakP-1600-times2-*',
        'lib/low/KeccakP-1600-times2/SIMD128/SSSE3-u2/SIMD128-config.h',
        'lib/low/KeccakP-1600-times4/AVX2/KeccakP-1600-times4-*',
        'lib/low/KeccakP-1600-times4/AVX2/u12/SIMD256-config.h',
        'lib/low/KeccakP-1600-times8/fallback-on4/*']:
        for source in xkcp_sources.glob(glob):
            shutil.copy(source, target_sha3)

    # copy tests
    for test_source in ("api_test.c",):
        shutil.copy(tools_sources / test_source, target_tests)
    # copy NIST files
    for tool_source in ("rng.c", "rng.h", "PQCgenKAT_sign.c"):
        shutil.copy(tools_sources / tool_source, target_nist_kat)
    for tool_source in ("Makefile",):
        shutil.copy(tools_sources / tool_source, target)

    # build and create KATs
    print(f"Building {param_name}")
    cpu_count = os.cpu_count()
    subprocess.check_call(
        ["make"] if cpu_count is None else ["make", "-j", str(max(2, cpu_count - 1))],
        cwd=target,
    )

    print(f"running api_test")
    subprocess.check_call(target_tests / "api_test", cwd=target_kat)

    print(f"Generating KATs for {param_name}")
    subprocess.check_call(target_nist_kat / "PQCgenKAT_sign", cwd=target_kat)

    subprocess.check_call(["make", "clean"], cwd=target)


def main():
    project_root = Path(sys.argv[1])
    build_root = Path(sys.argv[2])
    target_root = Path(sys.argv[3])
    param_names = sys.argv[4:]

    for param_name in param_names:
        generate(project_root, build_root, target_root, param_name)


if __name__ == "__main__":
    main()
