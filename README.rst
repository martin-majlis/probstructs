Probabilistic Structures
========================

`ProbStructs` as easy to use C++ library with probabilistic structures.

|build-status| |docs| |github-stars-flat|

Documentation
-------------

Full documentation is available at http://probstructs.readthedocs.io/en/latest/

Classes
-------

 * `CountMinSketch`_ - frequency table of events in a stream
 * `ExponentialHistorgram`_ - frequency of specific event in the last N elements from a stream
 * `ExponentialCountMinSketch`_ - frequency table of events in the last N elements from a stream
 * `Hash`_ - hashing function

.. _CountMinSketch: https://probstructs.readthedocs.io/en/latest/
.. _ExponentialHistorgram: https://probstructs.readthedocs.io/en/latest/
.. _ExponentialCountMinSketch: https://probstructs.readthedocs.io/en/latest/
.. _Hash: https://probstructs.readthedocs.io/en/latest/

Example
-------

.. code-block:: c++

    using namespace probstructs;

    ExponentialCountMinSketch<int> sketch(100, 4, 8);

    uint ts = 0;

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



.. |build-status| image:: https://travis-ci.org/martin-majlis/probstructs.svg?branch=master
    :alt: build status
    :target: https://travis-ci.org/martin-majlis/probstructs

.. |docs| image:: https://readthedocs.org/projects/probstructs/badge/?version=latest
    :target: http://probstructs.readthedocs.io/en/latest/?badge=latest
    :alt: Documentation Status

.. |github-stars-flat| image:: https://img.shields.io/github/stars/martin-majlis/probstructs.svg?style=flat&label=Stars
	:target: https://github.com/martin-majlis/probstructs/
	:alt: GitHub stars