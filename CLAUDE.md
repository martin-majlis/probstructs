# ProbStructs — Claude Code guidance

## Project overview

C++17 header-only library of probabilistic data structures:
- `CountMinSketch` — frequency table of events in a stream
- `ExponentialHistorgram` — frequency of a specific event in the last N elements
- `ExponentialCountMinSketch` — frequency table of events in the last N elements

Source lives in `probstructs/probstructs.h`. Tests are in `tests/`. Benchmarks are in `benchmarks/`.

## Git workflow

**Always open a PR — never push directly to `master`.**

## Build

```bash
make debug-build     # configure + compile (Debug)
make debug-test      # run unit tests (Debug)
make release-build   # configure + compile (Release)
```

CMake is auto-detected from PATH and common Homebrew locations (`/opt/homebrew/bin`, `/usr/local/bin`).

## Testing

Unit tests use **Google Benchmark** (fetched via FetchContent at configure time):

```bash
make debug-test      # shortcut for the above
# or directly:
cmake -S . -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -B _build
cmake --build _build
ctest --test-dir _build --output-on-failure
```

On Windows, ctest needs `-C Debug` because the VS generator is multi-config.

## Benchmarks

Google Benchmark suite, opt-in (`BUILD_BENCHMARKS=OFF` by default).
Requires **uv** for the comparison and regression scripts (fetches `numpy`/`scipy` automatically).

```bash
make bench-build     # configure + compile benchmarks
make bench-run       # run and save results to benchmark_results/local/<timestamp>.json
make bench-compare   # compare the two most-recent local results (uses uv)
```

Result directories:
- `benchmark_results/local/` — gitignored; local developer runs only
- `benchmark_results/ci/` — committed to master; CI regression baseline

`bench_compare.sh` calls Google Benchmark's `compare.py` via `uv run --with numpy --with scipy`.
`bench_check_regression.py` has PEP 723 metadata and is invoked via `uv run`.

## CI

Two workflows in `.github/workflows/`:
- `benchmark.yml` — benchmark regression check (Ubuntu only). Benchmarks run with `--benchmark_repetitions=3`. Two rules: fail if ≥2 benchmarks are >10% slower (per-benchmark), OR if the geomean regresses >5% (geomean). On pushes to `master` commits new results as the next baseline.
- `test.yml` — unit tests on Linux/macOS/Windows.
- `build.yml` — legacy build/artifact workflow using `nicledomaS/cmake_build_action`; tests are disabled there because `ci.yml` covers all platforms.

## Code conventions

- C++17; use `uint32_t` (not `uint` — MSVC doesn't define `uint`)
- Dependencies fetched via CMake FetchContent (fmtlib, googletest, googlebenchmark)
- No external runtime dependencies
