#!/usr/bin/env bash
set -euo pipefail

RESULTS_DIR="benchmark_results/local"
COMPARE_PY_SEARCH_PATHS=(
    "_bench/_deps/googlebenchmark-src/tools/compare.py"
    "_debug/_deps/googlebenchmark-src/tools/compare.py"
    "_release/_deps/googlebenchmark-src/tools/compare.py"
)

COMPARE_PY=""
for p in "${COMPARE_PY_SEARCH_PATHS[@]}"; do
    if [[ -f "${p}" ]]; then
        COMPARE_PY="${p}"
        break
    fi
done

if [[ -z "${COMPARE_PY}" ]]; then
    echo "ERROR: Could not find Google Benchmark compare.py."
    echo "Run 'make bench-configure' first to fetch the dependency."
    exit 1
fi

if [[ $# -ge 2 ]]; then
    FILE1="${1}"
    FILE2="${2}"
else
    SORTED=($(ls -t "${RESULTS_DIR}"/*.json 2>/dev/null))
    if [[ ${#SORTED[@]} -lt 2 ]]; then
        echo "ERROR: Need at least 2 result files in ${RESULTS_DIR}/."
        echo "Run 'make bench-run' at least twice, or pass two filenames explicitly."
        exit 1
    fi
    FILE2="${SORTED[0]}"
    FILE1="${SORTED[1]}"
    echo "Comparing:"
    echo "  baseline : ${FILE1}"
    echo "  candidate: ${FILE2}"
fi

python3 "${COMPARE_PY}" benchmarks "${FILE1}" "${FILE2}"
