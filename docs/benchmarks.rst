Benchmarks
==========

ProbStructs ships a `Google Benchmark`_ suite that measures ``inc`` and ``get``
performance for all three data structures across a range of parameter sizes.
Results are saved as JSON so you can track performance across commits.

Result storage
--------------

Results are kept in two separate directories so local and CI runs never mix:

* ``benchmark_results/local/`` — your local runs; **gitignored** (never committed)
* ``benchmark_results/ci/`` — CI runs committed to ``master``; used as the
  regression baseline for pull requests

``make bench-run`` and ``make bench-compare`` always target ``local/``.
The CI workflow saves to ``ci/`` automatically.

.. _Google Benchmark: https://github.com/google/benchmark

Prerequisites
-------------

You need **CMake 3.11+** and a C++17-capable compiler.  The Makefile finds
cmake automatically in ``PATH`` and the common Homebrew locations
(``/opt/homebrew/bin``, ``/usr/local/bin``).

macOS:

.. code-block:: bash

    brew install cmake

Ubuntu/Debian:

.. code-block:: bash

    # Kitware's APT repository provides a current release:
    # https://apt.kitware.com/
    sudo apt-get install cmake

Building
--------

Benchmarks are **off by default** and live in their own CMake build directory
``_bench`` so they never slow down a normal debug or release build.

.. code-block:: bash

    make bench-build

This runs ``cmake`` with ``-DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON``,
fetches Google Benchmark via ``FetchContent``, and compiles
``_bench/benchmarks/probstructs_benchmark``.

Running
-------

.. code-block:: bash

    make bench-run

The script ``scripts/bench_run.sh`` executes the binary and writes a
timestamped JSON file to ``benchmark_results/``::

    benchmark_results/2026-05-27_14-30-00.json

The file is committed to the repository so the run history is preserved.

You can pass extra Google Benchmark flags after the target:

.. code-block:: bash

    ./scripts/bench_run.sh _bench/benchmarks/probstructs_benchmark \
        --benchmark_filter=CountMin \
        --benchmark_repetitions=5

Comparing runs
--------------

After two or more runs:

.. code-block:: bash

    make bench-compare

This calls ``scripts/bench_compare.sh``, which locates Google Benchmark's
``tools/compare.py`` inside the ``_bench`` build directory and compares the
two most-recent result files automatically.

To compare specific files:

.. code-block:: bash

    ./scripts/bench_compare.sh \
        benchmark_results/2026-05-20_10-00-00.json \
        benchmark_results/2026-05-27_14-30-00.json

The comparison output shows the relative change (``+``/``-``) for each
benchmark, making regressions easy to spot.

What is benchmarked
-------------------

Key domains
~~~~~~~~~~~

All benchmarks use pre-generated key pools and access sequences to avoid
measuring string construction inside the timed loop:

.. list-table::
   :header-rows: 1
   :widths: 20 15 65

   * - Name
     - Keys
     - Purpose
   * - Small
     - 1 000
     - High collision rate; sketch data fits in L1/L2 cache
   * - Medium
     - 10 000
     - Moderate collisions; L2/L3 resident
   * - Large
     - 100 000
     - Low collision rate; exercises cache pressure

Access distributions
~~~~~~~~~~~~~~~~~~~~

* **Uniform** — every key equally likely (worst-case for accuracy, neutral for cache).
* **Zipf (s=1)** — top 1 % of keys receive ~50 % of traffic.  Realistic for
  web requests, event streams, and user activity logs.  Hot keys stay warm in
  cache, which can reveal differences invisible under uniform load.

CountMinSketch
~~~~~~~~~~~~~~

Benchmarks: ``Inc``, ``Get``, ``Mixed`` (80 % inc / 20 % get).

Parameters swept: sketch width (500 – 5 000), depth (3 – 7), key domain
(1 K – 100 K), distribution (uniform / Zipf).  The full sketch size
(``width × depth × 4`` bytes) spans from well within L1 cache to L3-resident,
making the parameter sweep a de-facto cache-pressure study.

ExponentialHistorgram
~~~~~~~~~~~~~~~~~~~~~

Benchmarks: ``Inc``, ``Get``.

The key variable is **tick_step** — how far the stream clock advances between
calls.  This controls how much internal bucket-cascade work each call triggers:

.. list-table::
   :header-rows: 1
   :widths: 20 20 60

   * - Mode
     - tick_step
     - Meaning
   * - Steady
     - 1
     - Minimal reshuffling — hot path for continuous streams
   * - Bursty
     - window / 4
     - Several buckets shift per call
   * - Expiry
     - window / 2
     - Majority of stored data expires on every call

Windows tested: 256, 1 024, 16 384.

ExponentialCountMinSketch
~~~~~~~~~~~~~~~~~~~~~~~~~

Benchmarks: ``Inc``, ``Get``, ``Mixed`` (80 % inc / 20 % get).

Parameters swept: sketch width (500 – 2 000), depth (3 – 5), window
(256 – 4 096), tick_step (steady / bursty), key domain (1 K – 100 K),
distribution (uniform / Zipf).

End-to-end scenarios
~~~~~~~~~~~~~~~~~~~~

These simulate realistic workloads rather than isolating a single operation:

**FrequencyEstimation** — 90 % inserts / 10 % point queries over a 100 K Zipf
key domain.  Models event counting (URL hits, product views).

**HeavyHitterDetection** — Sliding-window frequency counting over a 10 K Zipf
domain.  Every 10 inserts the current key is queried; keys exceeding 1 % of
the window are flagged.  Reports ``heavy_hitters_pct`` as a counter.

**RateLimiter** — Per-client request rate checking against a 60-second sliding
window.  Each event is either counted (if under the rate limit) or dropped.
Models API rate limiting and quota enforcement.  Reports ``throttled_pct``.

All benchmarks report **items/s** throughput in addition to wall time.

Continuous integration
----------------------

The ``CI`` GitHub Actions workflow (``.github/workflows/ci.yml``) runs
automatically on every pull request and push to ``master``:

* **Unit tests** — built and run on Ubuntu and macOS.  A failure blocks the PR.

* **Benchmark regression check** — benchmarks are built and run on a fixed
  Ubuntu runner for consistency.  The latest JSON file committed to ``master``
  is used as the baseline.  If any benchmark is more than **5 %** slower the
  workflow fails and the PR is blocked.

* **Baseline update** — on pushes to ``master`` the new result file is
  automatically committed to ``benchmark_results/`` so future PRs always
  compare against up-to-date hardware measurements.

.. note::

   GitHub Actions runners are virtualised.  Timing can vary by ±2–3 % between
   runs.  The 5 % threshold provides headroom for that noise.  If the check
   becomes flaky on your setup, widen it by changing the ``--threshold``
   argument in the workflow file.

You can also run the regression check locally::

    python3 scripts/bench_check_regression.py \
        --baseline benchmark_results/2026-05-20_10-00-00.json \
        --current  benchmark_results/2026-05-27_14-30-00.json \
        --threshold 5

CMake option reference
----------------------

``BUILD_BENCHMARKS``
    Set to ``ON`` to include the ``benchmarks/`` subdirectory.
    Default: ``OFF``.

``MODERN_CMAKE_BUILD_BENCHMARKS``
    Emergency override to enable benchmarks when ProbStructs is consumed
    via ``add_subdirectory`` from another project.
    Default: not set.
