Classes
=======

.. _clsCountMinSketch:

CountMinSketch
--------------

Documentation
^^^^^^^^^^^^^
.. doxygenclass:: probstructs::CountMinSketch
   :members:

Example
^^^^^^^

.. code-block:: c++

    using namespace probstructs;

    CountMinSketch<int> sketch(100, 4);
    sketch.inc("aaa", 1);
    sketch.inc(std::string("bbb"), 5);
    sketch.inc("aaa", 2);

    std::cerr << sketch.get(std::string("aaa") << std::endl;
    // 3

    std::cerr << sketch.get("bbb") << std::endl;
    // 5

    std::cerr << sketch.get("ccc") << std::endl;
    // 0

.. _clsExponentialHistorgram:

ExponentialHistorgram
---------------------

Documentation
^^^^^^^^^^^^^

.. doxygenclass:: probstructs::ExponentialHistorgram
   :members:


Example
^^^^^^^

.. code-block:: c++

    using namespace probstructs;

    ExponentialHistorgram<int> eh(4);
    uint ts = 0;

    ts = 0;
    std::cerr << eh.get(1, ts) << ", " << eh.get(4, ts) << ", " << eh.get(8, ts) << std::endl;
    // 0, 0, 0

    eh.inc(ts, 1);

    std::cerr << eh.get(1, ts) << ", " << eh.get(4, ts) << ", " << eh.get(8, ts) << std::endl;
    // 1, 1, 1

    ts = 1;
    std::cerr << eh.get(1, ts) << ", " << eh.get(4, ts) << ", " << eh.get(8, ts) << std::endl;
    // 0, 1, 1

    eh.inc(ts, 1);

    std::cerr << eh.get(1, ts) << ", " << eh.get(4, ts) << ", " << eh.get(8, ts) << std::endl;
    // 1, 2, 2

    ts = 3;
    std::cerr << eh.get(1, ts) << ", " << eh.get(4, ts) << ", " << eh.get(8, ts) << std::endl;
    // 0, 2, 2

    eh.inc(ts, 1);
    std::cerr << eh.get(1, ts) << ", " << eh.get(4, ts) << ", " << eh.get(8, ts) << std::endl;
    // 1, 3, 3

    ts = 5;
    std::cerr << eh.get(1, ts) << ", " << eh.get(4, ts) << ", " << eh.get(8, ts) << std::endl;
    // 0, 1, 1

    eh.inc(ts, 1);
    std::cerr << eh.get(1, ts) << ", " << eh.get(4, ts) << ", " << eh.get(8, ts) << std::endl;
    // 1, 2, 2


.. _clsExponentialCountMinSketch:

ExponentialCountMinSketch
-------------------------

Documentation
^^^^^^^^^^^^^

.. doxygenclass:: probstructs::ExponentialCountMinSketch
   :members:

Example
^^^^^^^

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


.. _clsHash:

Hash
----

Documentation
^^^^^^^^^^^^^

.. doxygenclass:: probstructs::Hash
   :members:

Example
^^^^^^^

.. code-block:: c++

    using namespace probstructs;
    Hash h1(1);

    std::cerr << h1.hash("aaa") << std::endl;
    // 390644701
    std::cerr << h1.hash(std::string("bbb")) << std::endl;
    // 2512199470
