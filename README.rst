Probabilistic Structures
========================

`ProbStructs` as easy to use C++ library with probabilistic structures.

|build-status| |test-status| |benchmark-status| |docs| |github-stars-flat|

Documentation
-------------

Full documentation is available at http://probstructs.readthedocs.io/en/latest/

Classes
-------

* `CountMinSketch`_ - frequency table of events in a stream
* `ExponentialHistorgram`_ - frequency of specific event in the last N elements from a stream
* `ExponentialCountMinSketch`_ - frequency table of events in the last N elements from a stream
* `Hash`_ - hashing function

.. _CountMinSketch: https://probstructs.readthedocs.io/en/latest/classes.html#countminsketch
.. _ExponentialHistorgram: https://probstructs.readthedocs.io/en/latest/classes.html#exponentialhistorgram
.. _ExponentialCountMinSketch: https://probstructs.readthedocs.io/en/latest/classes.html#exponentialcountminsketch
.. _Hash: https://probstructs.readthedocs.io/en/latest/classes.html#hash

Example
-------

.. code-block:: c++

    using namespace probstructs;

    ExponentialCountMinSketch<int> sketch(100, 4, 8);

    uint32_t ts = 0;

    ts = 0;
    sketch.inc("aaa", ts, 1);
    sketch.inc(std::string("bbb"), ts, 4);
    sketch.inc("ccc", ts, 8);

    std::cerr << sketch.get(std::string("aaa"), 4, ts) << std::endl;
    // 1

    std::cerr << sketch.get("bbb", 4, ts) << std::endl;
    // 4

    std::cerr << sketch.get("ccc", 4, ts) << std::endl;
    // 8

    std::cerr << sketch.get("ddd", 4, ts) << std::endl;
    // 0

    ts = 4;
    std::cerr << sketch.get("aaa", 2, ts) << std::endl;
    // 0
    std::cerr << sketch.get("bbb", 2, ts) << std::endl;
    // 0
    std::cerr << sketch.get(std::string("ccc"), 2, ts) << std::endl;
    // 0
    std::cerr << sketch.get("ddd", 2, ts) << std::endl;
    // 0

    std::cerr << sketch.get("aaa", 8, ts) << std::endl;
    // 1
    std::cerr << sketch.get("bbb", 8, ts) << std::endl;
    // 4
    std::cerr << sketch.get("ccc", 8, ts) << std::endl;
    // 8
    std::cerr << sketch.get("ddd", 8, ts) << std::endl;
    // 0


Building the docs
-----------------

Prerequisites: **CMake 3.11+**, **Doxygen**, **Graphviz**, and the Python
packages listed in ``docs/requirements.txt`` (``breathe`` and
``sphinx-rtd-theme``).

macOS:

.. code-block:: bash

    brew install cmake doxygen graphviz
    pip install -r docs/requirements.txt

Ubuntu/Debian:

.. code-block:: bash

    sudo apt-get install cmake doxygen graphviz
    pip install -r docs/requirements.txt

Then build:

.. code-block:: bash

    make docs-build

The generated HTML lands in ``_docs/docs/sphinx/``.

Benchmarks
----------

Build and run the benchmark suite (requires CMake 3.11+ and a C++17 compiler;
install on macOS with ``brew install cmake``):

.. code-block:: bash

    make bench-build    # fetches Google Benchmark, compiles
    make bench-run      # runs and saves results to benchmark_results/local/<timestamp>.json
    make bench-compare  # compares the two most-recent local result files

Results are stored in ``benchmark_results/local/`` (gitignored, per-machine)
so local runs never pollute the repo. CI results are committed to
``benchmark_results/ci/`` and used as the regression baseline for pull requests.

See the `full benchmark documentation`_ for details on filtering, repeating
runs, and comparing specific result files.

.. _full benchmark documentation: https://probstructs.readthedocs.io/en/latest/benchmarks.html

.. |build-status| image:: https://github.com/martin-majlis/probstructs/actions/workflows/build.yml/badge.svg
    :alt: Build status
    :target: https://github.com/martin-majlis/probstructs/actions/workflows/build.yml

.. |test-status| image:: https://github.com/martin-majlis/probstructs/actions/workflows/test.yml/badge.svg
    :alt: Test status
    :target: https://github.com/martin-majlis/probstructs/actions/workflows/test.yml

.. |benchmark-status| image:: https://github.com/martin-majlis/probstructs/actions/workflows/benchmark.yml/badge.svg
    :alt: Benchmark status
    :target: https://github.com/martin-majlis/probstructs/actions/workflows/benchmark.yml

.. |docs| image:: https://readthedocs.org/projects/probstructs/badge/?version=latest
    :target: http://probstructs.readthedocs.io/en/latest/?badge=latest
    :alt: Documentation Status

.. |github-stars-flat| image:: https://img.shields.io/github/stars/martin-majlis/probstructs.svg?style=flat&label=Stars
	:target: https://github.com/martin-majlis/probstructs/
	:alt: GitHub stars
