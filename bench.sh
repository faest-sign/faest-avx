#!/bin/sh

set -eu
set -x

now="$(date '+%s')"
host="$(uname -n)"
ref="$(git rev-parse HEAD)"

prefix="bench_${ref}_${host}_${now}"

build_dir="build_bench"

configure_log="${prefix}_configure.log"
build_log="${prefix}_build.log"
benchmark_out_v2="${prefix}_bench_v2.xml"
benchmark_out_v3="${prefix}_bench_v3.xml"
archive_out="${prefix}.tar.gz"

echo "[+] Configuring build"
meson setup "${build_dir}" --buildtype=release -Db_lto=true -Doptimization=3 | tee "${configure_log}"

echo "[+] Compiling"
meson compile -C "${build_dir}" -v | tee "${build_log}"

echo "[+] Running tests"
"./${build_dir}/test/tests"

echo "[+] Sleeping for 30s"
sleep 30s

if command -v taskset >/dev/null 2>&1; then
    echo "[+] Running v3 benchmarks"
    taskset -c 0 "./${build_dir}/test/bench" 'bench variants - v3::*' --order lex --benchmark-samples 1000 --reporter XML > "${benchmark_out_v3}"
    echo "[+] Running v2 benchmarks"
    taskset -c 0 "./${build_dir}/test/bench" 'bench variants - v2::*' --order lex --benchmark-samples 1000 --reporter XML > "${benchmark_out_v2}"
else
    echo "[+] Running v3 benchmarks"
    "./${build_dir}/test/bench" 'bench variants - v3::*' --order lex --benchmark-samples 1000 --reporter XML > "${benchmark_out_v3}"
    echo "[+] Running v2 benchmarks"
    "./${build_dir}/test/bench" 'bench variants - v2::*' --order lex --benchmark-samples 1000 --reporter XML > "${benchmark_out_v2}"
fi

echo "[+] Writing data and logs into '${archive_out}'"
tar vczf "${archive_out}" \
    "${configure_log}" \
    "${build_log}" \
    "${benchmark_out_v2}" \
    "${benchmark_out_v3}"
