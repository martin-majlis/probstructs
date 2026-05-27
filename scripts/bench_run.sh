#!/usr/bin/env bash
set -euo pipefail

BENCH_BIN="${1:-_bench/benchmarks/probstructs_benchmark}"
RESULTS_DIR="benchmark_results/local"

mkdir -p "${RESULTS_DIR}"

TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
OUTPUT_FILE="${RESULTS_DIR}/${TIMESTAMP}.json"

echo "Running benchmarks: ${BENCH_BIN}"
echo "Output: ${OUTPUT_FILE}"

"${BENCH_BIN}" \
    --benchmark_out="${OUTPUT_FILE}" \
    --benchmark_out_format=json \
    --benchmark_color=true \
    "${@:2}"

echo ""
echo "Results saved to: ${OUTPUT_FILE}"
