Benchmarks
==========

ProbStructs ships a `Google Benchmark`_ suite that measures ``inc`` and ``get``
performance for all three data structures across a range of parameter sizes.
Results are saved as JSON so you can track performance across commits.

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

Each data structure is benchmarked for both ``inc`` (insert) and ``get``
(query) with three parameter sizes:

.. list-table::
   :header-rows: 1
   :widths: 30 20 20 20 10

   * - Structure
     - Small
     - Medium
     - Large
     - Operations
   * - ``CountMinSketch``
     - width=100, depth=3
     - width=1000, depth=5
     - width=10000, depth=7
     - inc, get
   * - ``ExponentialHistorgram``
     - window=64
     - window=1024
     - window=16384
     - inc, get
   * - ``ExponentialCountMinSketch``
     - width=100, depth=3, window=64
     - width=1000, depth=5, window=1024
     - width=5000, depth=7, window=4096
     - inc, get

All benchmarks report **items/s** throughput in addition to wall time.

CMake option reference
----------------------

``BUILD_BENCHMARKS``
    Set to ``ON`` to include the ``benchmarks/`` subdirectory.
    Default: ``OFF``.

``MODERN_CMAKE_BUILD_BENCHMARKS``
    Emergency override to enable benchmarks when ProbStructs is consumed
    via ``add_subdirectory`` from another project.
    Default: not set.
