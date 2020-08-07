Welcome to ProbStructs's documentation!
=======================================

`ProbStructs` library provides probabilistic structures to count elements in the stream.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   classes

:ref:`genindex`

Classes
-------

 * :ref:`clsCountMinSketch` - frequency table of events in a stream
 * :ref:`clsExponentialHistorgram` - frequency of specific event in the last N elements from a stream
 * :ref:`clsExponentialCountMinSketch` - frequency table of events in the last N elements from a stream
 * :ref:`clsHash` - hashing function

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

Wrappers
--------

* Python - https://github.com/martin-majlis/py-probstructs/
