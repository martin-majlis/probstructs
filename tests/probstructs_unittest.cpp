#include <limits.h>
#include "../src/probstructs.h"
#include "gtest/gtest.h"
namespace {

TEST(HashTest, Simple) {
    Hash h1(1);
    EXPECT_EQ(390644701,h1.hash(std::string("aaa")));
    EXPECT_EQ(2512199470,h1.hash(std::string("bbb")));


    Hash h2(2);
    EXPECT_EQ(2275761540,h2.hash("aaa"));
    EXPECT_EQ(2714074101,h2.hash("bbb"));
}

TEST(CountMinSketchTest, Simple) {
    CountMinSketch<int> sketch(100, 4);
    sketch.inc("aaa", 1);
    sketch.inc(std::string("bbb"), 5);
    sketch.inc("aaa", 2);

    EXPECT_EQ(3,sketch.get(std::string("aaa")));
    EXPECT_EQ(5,sketch.get("bbb"));
    EXPECT_EQ(0,sketch.get("ccc"));
}

TEST(ExponentialHistogramTest, SimpleTick0) {
    ExponentialHistorgram<int> eh(1);
    // it has to be empty at the beginning
    EXPECT_EQ(0,eh.get(1, 0));
    eh.inc(0, 1);
    EXPECT_EQ(1,eh.get(1, 0));
    eh.inc(0, 1);
    EXPECT_EQ(2,eh.get(1, 0));
    eh.inc(0, 2);
    EXPECT_EQ(4,eh.get(1, 0));
}

TEST(ExponentialHistogramTest, SimpleTick1) {
    ExponentialHistorgram<int> eh(1);
    // it has to be empty at the beginning
    EXPECT_EQ(0,eh.get(1, 1));
    eh.inc(1, 1);
    EXPECT_EQ(1,eh.get(1, 1));
    eh.inc(1, 1);
    EXPECT_EQ(2,eh.get(1, 1));
    eh.inc(1, 2);
    EXPECT_EQ(4,eh.get(1, 1));
}

TEST(ExponentialHistogramTest, Expire1Size) {
    ExponentialHistorgram<int> eh(1);
    // insert at ts = 1
    eh.inc(1, 1);
    EXPECT_EQ(1,eh.get(1, 1));
    eh.inc(1, 1);
    EXPECT_EQ(2,eh.get(1, 1));

    // insert at ts = 2
    eh.inc(2, 1);
    EXPECT_EQ(1,eh.get(1, 2));
}

TEST(ExponentialHistogramTest, Expire8Size) {
    ExponentialHistorgram<int> eh(8);
    uint ts = 0;

    // TS = 0
    ts = 0;
    EXPECT_EQ(0,eh.get(1, ts));
    EXPECT_EQ(0,eh.get(2, ts));
    EXPECT_EQ(0,eh.get(3, ts));
    EXPECT_EQ(0,eh.get(4, ts));
    EXPECT_EQ(0,eh.get(5, ts));
    EXPECT_EQ(0,eh.get(6, ts));
    EXPECT_EQ(0,eh.get(7, ts));
    EXPECT_EQ(0,eh.get(8, ts));
    EXPECT_EQ(0,eh.get(9, ts));

    eh.inc(ts, 1);
    EXPECT_EQ(1,eh.get(1, ts));
    EXPECT_EQ(1,eh.get(2, ts));
    EXPECT_EQ(1,eh.get(3, ts));
    EXPECT_EQ(1,eh.get(4, ts));
    EXPECT_EQ(1,eh.get(5, ts));
    EXPECT_EQ(1,eh.get(6, ts));
    EXPECT_EQ(1,eh.get(7, ts));
    EXPECT_EQ(1,eh.get(8, ts));
    EXPECT_EQ(1,eh.get(9, ts));

    // TS = 1
    ts = 1;
    EXPECT_EQ(0,eh.get(1, ts));
    EXPECT_EQ(1,eh.get(2, ts));
    EXPECT_EQ(1,eh.get(3, ts));
    EXPECT_EQ(1,eh.get(4, ts));
    EXPECT_EQ(1,eh.get(5, ts));
    EXPECT_EQ(1,eh.get(6, ts));
    EXPECT_EQ(1,eh.get(7, ts));
    EXPECT_EQ(1,eh.get(8, ts));
    EXPECT_EQ(1,eh.get(9, ts));

    eh.inc(ts, 1);
    EXPECT_EQ(1,eh.get(1, ts));
    EXPECT_EQ(2,eh.get(2, ts));
    EXPECT_EQ(2,eh.get(3, ts));
    EXPECT_EQ(2,eh.get(4, ts));
    EXPECT_EQ(2,eh.get(5, ts));
    EXPECT_EQ(2,eh.get(6, ts));
    EXPECT_EQ(2,eh.get(7, ts));
    EXPECT_EQ(2,eh.get(8, ts));
    EXPECT_EQ(2,eh.get(9, ts));

    // TS = 2
    ts = 2;
    EXPECT_EQ(0,eh.get(1, ts));
    EXPECT_EQ(1,eh.get(2, ts));
    EXPECT_EQ(2,eh.get(3, ts));
    EXPECT_EQ(2,eh.get(4, ts));
    EXPECT_EQ(2,eh.get(5, ts));
    EXPECT_EQ(2,eh.get(6, ts));
    EXPECT_EQ(2,eh.get(7, ts));
    EXPECT_EQ(2,eh.get(8, ts));
    EXPECT_EQ(2,eh.get(9, ts));

    eh.inc(ts, 1);
    EXPECT_EQ(1,eh.get(1, ts));
    EXPECT_EQ(2,eh.get(2, ts));
    EXPECT_EQ(3,eh.get(3, ts));
    EXPECT_EQ(3,eh.get(4, ts));
    EXPECT_EQ(3,eh.get(5, ts));
    EXPECT_EQ(3,eh.get(6, ts));
    EXPECT_EQ(3,eh.get(7, ts));
    EXPECT_EQ(3,eh.get(8, ts));
    EXPECT_EQ(3,eh.get(9, ts));

    // TS = 3
    ts = 3;
    EXPECT_EQ(0,eh.get(1, ts));
    EXPECT_EQ(1,eh.get(2, ts));
    EXPECT_EQ(2,eh.get(3, ts));
    EXPECT_EQ(3,eh.get(4, ts));
    EXPECT_EQ(3,eh.get(5, ts));
    EXPECT_EQ(3,eh.get(6, ts));
    EXPECT_EQ(3,eh.get(7, ts));
    EXPECT_EQ(3,eh.get(8, ts));
    EXPECT_EQ(3,eh.get(9, ts));

    eh.inc(ts, 1);
    EXPECT_EQ(1,eh.get(1, ts));
    EXPECT_EQ(2,eh.get(2, ts));
    EXPECT_EQ(3,eh.get(3, ts));
    EXPECT_EQ(4,eh.get(4, ts));
    EXPECT_EQ(4,eh.get(5, ts));
    EXPECT_EQ(4,eh.get(6, ts));
    EXPECT_EQ(4,eh.get(7, ts));
    EXPECT_EQ(4,eh.get(8, ts));
    EXPECT_EQ(4,eh.get(9, ts));

    // TS = 4
    ts = 4;
    EXPECT_EQ(0,eh.get(1, ts));
    EXPECT_EQ(1,eh.get(2, ts));
    EXPECT_EQ(2,eh.get(3, ts));
    EXPECT_EQ(3,eh.get(4, ts));
    EXPECT_EQ(4,eh.get(5, ts));
    EXPECT_EQ(4,eh.get(6, ts));
    EXPECT_EQ(4,eh.get(7, ts));
    EXPECT_EQ(4,eh.get(8, ts));
    EXPECT_EQ(4,eh.get(9, ts));

    eh.inc(ts, 1);
    EXPECT_EQ(1,eh.get(1, ts));
    EXPECT_EQ(2,eh.get(2, ts));
    EXPECT_EQ(3,eh.get(3, ts));
    EXPECT_EQ(4,eh.get(4, ts));
    EXPECT_EQ(5,eh.get(5, ts));
    EXPECT_EQ(5,eh.get(6, ts));
    EXPECT_EQ(5,eh.get(7, ts));
    EXPECT_EQ(5,eh.get(8, ts));
    EXPECT_EQ(5,eh.get(9, ts));

    // TS = 5
    ts = 5;
    EXPECT_EQ(0,eh.get(1, ts));
    EXPECT_EQ(1,eh.get(2, ts));
    EXPECT_EQ(2,eh.get(3, ts));
    EXPECT_EQ(3,eh.get(4, ts));
    EXPECT_EQ(4,eh.get(5, ts));
    EXPECT_EQ(4,eh.get(6, ts)); // *
    EXPECT_EQ(5,eh.get(7, ts));
    EXPECT_EQ(5,eh.get(8, ts));
    EXPECT_EQ(5,eh.get(9, ts));

    eh.inc(ts, 1);
    EXPECT_EQ(1,eh.get(1, ts));
    EXPECT_EQ(2,eh.get(2, ts));
    EXPECT_EQ(3,eh.get(3, ts));
    EXPECT_EQ(4,eh.get(4, ts));
    EXPECT_EQ(5,eh.get(5, ts));
    EXPECT_EQ(5,eh.get(6, ts)); // *
    EXPECT_EQ(6,eh.get(7, ts));
    EXPECT_EQ(6,eh.get(8, ts));
    EXPECT_EQ(6,eh.get(9, ts));

    // TS = 6
    ts = 6;
    EXPECT_EQ(0,eh.get(1, ts));
    EXPECT_EQ(1,eh.get(2, ts));
    EXPECT_EQ(2,eh.get(3, ts));
    EXPECT_EQ(3,eh.get(4, ts));
    EXPECT_EQ(4,eh.get(5, ts));
    EXPECT_EQ(5,eh.get(6, ts));
    EXPECT_EQ(5,eh.get(7, ts)); // *
    EXPECT_EQ(6,eh.get(8, ts));
    EXPECT_EQ(6,eh.get(9, ts));

    eh.inc(ts, 1);
    EXPECT_EQ(1,eh.get(1, ts));
    EXPECT_EQ(2,eh.get(2, ts));
    EXPECT_EQ(3,eh.get(3, ts));
    EXPECT_EQ(4,eh.get(4, ts));
    EXPECT_EQ(5,eh.get(5, ts));
    EXPECT_EQ(6,eh.get(6, ts));
    EXPECT_EQ(6,eh.get(7, ts)); // *
    EXPECT_EQ(7,eh.get(8, ts));
    EXPECT_EQ(7,eh.get(9, ts));

    // TS = 7
    ts = 7;
    EXPECT_EQ(0,eh.get(1, ts));
    EXPECT_EQ(1,eh.get(2, ts));
    EXPECT_EQ(2,eh.get(3, ts));
    EXPECT_EQ(3,eh.get(4, ts));
    EXPECT_EQ(4,eh.get(5, ts));
    EXPECT_EQ(5,eh.get(6, ts));
    EXPECT_EQ(5,eh.get(7, ts)); // *
    EXPECT_EQ(6,eh.get(8, ts)); // *
    EXPECT_EQ(6,eh.get(9, ts)); // *

    eh.inc(ts, 1);
    EXPECT_EQ(1,eh.get(1, ts));
    EXPECT_EQ(2,eh.get(2, ts));
    EXPECT_EQ(3,eh.get(3, ts));
    EXPECT_EQ(4,eh.get(4, ts));
    EXPECT_EQ(5,eh.get(5, ts));
    EXPECT_EQ(6,eh.get(6, ts));
    EXPECT_EQ(6,eh.get(7, ts)); // *
    EXPECT_EQ(7,eh.get(8, ts)); // *
    EXPECT_EQ(7,eh.get(9, ts)); // * ??

    // TS = 8
    ts = 8;
    EXPECT_EQ(0,eh.get(1, ts));
    EXPECT_EQ(1,eh.get(2, ts));
    EXPECT_EQ(2,eh.get(3, ts));
    EXPECT_EQ(3,eh.get(4, ts));
    EXPECT_EQ(4,eh.get(5, ts));
    EXPECT_EQ(5,eh.get(6, ts));
    EXPECT_EQ(6,eh.get(7, ts));
    EXPECT_EQ(6,eh.get(8, ts)); // *
    EXPECT_EQ(6,eh.get(9, ts)); // * ??

    eh.inc(ts, 1);
    EXPECT_EQ(1,eh.get(1, ts));
    EXPECT_EQ(2,eh.get(2, ts));
    EXPECT_EQ(3,eh.get(3, ts));
    EXPECT_EQ(4,eh.get(4, ts));
    EXPECT_EQ(5,eh.get(5, ts));
    EXPECT_EQ(6,eh.get(6, ts));
    EXPECT_EQ(7,eh.get(7, ts));
    EXPECT_EQ(7,eh.get(8, ts)); // *
    EXPECT_EQ(7,eh.get(9, ts)); // * ??

    // TS = 9
    ts = 9;
    EXPECT_EQ(0,eh.get(1, ts));
    EXPECT_EQ(1,eh.get(2, ts));
    EXPECT_EQ(2,eh.get(3, ts));
    EXPECT_EQ(3,eh.get(4, ts));
    EXPECT_EQ(4,eh.get(5, ts));
    EXPECT_EQ(5,eh.get(6, ts));
    EXPECT_EQ(6,eh.get(7, ts));
    EXPECT_EQ(7,eh.get(8, ts));
    EXPECT_EQ(7,eh.get(9, ts)); // * ??

    eh.inc(ts, 1);
    EXPECT_EQ(1,eh.get(1, ts));
    EXPECT_EQ(2,eh.get(2, ts));
    EXPECT_EQ(3,eh.get(3, ts));
    EXPECT_EQ(4,eh.get(4, ts));
    EXPECT_EQ(5,eh.get(5, ts));
    EXPECT_EQ(6,eh.get(6, ts));
    EXPECT_EQ(7,eh.get(7, ts));
    EXPECT_EQ(8,eh.get(8, ts));
    EXPECT_EQ(8,eh.get(9, ts));
}

TEST(ExponentialHistogramTest, Expire8SizeMediumJump) {
    ExponentialHistorgram<int> eh(8);
    uint ts = 0;

    // TS = 0
    ts = 0;
    EXPECT_EQ(0, eh.get(1, ts));
    EXPECT_EQ(0, eh.get(2, ts));
    EXPECT_EQ(0, eh.get(3, ts));
    EXPECT_EQ(0, eh.get(4, ts));
    EXPECT_EQ(0, eh.get(5, ts));
    EXPECT_EQ(0, eh.get(6, ts));
    EXPECT_EQ(0, eh.get(7, ts));
    EXPECT_EQ(0, eh.get(8, ts));
    EXPECT_EQ(0, eh.get(9, ts));

    eh.inc(ts, 1);
    EXPECT_EQ(1, eh.get(1, ts));
    EXPECT_EQ(1, eh.get(2, ts));
    EXPECT_EQ(1, eh.get(3, ts));
    EXPECT_EQ(1, eh.get(4, ts));
    EXPECT_EQ(1, eh.get(5, ts));
    EXPECT_EQ(1, eh.get(6, ts));
    EXPECT_EQ(1, eh.get(7, ts));
    EXPECT_EQ(1, eh.get(8, ts));
    EXPECT_EQ(1, eh.get(9, ts));

    // TS = 5
    ts = 5;
    EXPECT_EQ(0, eh.get(1, ts));
    EXPECT_EQ(0, eh.get(2, ts));
    EXPECT_EQ(0, eh.get(3, ts));
    EXPECT_EQ(0, eh.get(4, ts));
    EXPECT_EQ(1, eh.get(5, ts));
    EXPECT_EQ(1, eh.get(6, ts));
    EXPECT_EQ(1, eh.get(7, ts));
    EXPECT_EQ(1, eh.get(8, ts));
    EXPECT_EQ(1, eh.get(9, ts));
}

TEST(ExponentialHistogramTest, Expire8SizeLargeJump) {
    ExponentialHistorgram<int> eh(8);
    uint ts = 0;

    // TS = 0
    ts = 0;
    EXPECT_EQ(0, eh.get(1, ts));
    EXPECT_EQ(0, eh.get(2, ts));
    EXPECT_EQ(0, eh.get(3, ts));
    EXPECT_EQ(0, eh.get(4, ts));
    EXPECT_EQ(0, eh.get(5, ts));
    EXPECT_EQ(0, eh.get(6, ts));
    EXPECT_EQ(0, eh.get(7, ts));
    EXPECT_EQ(0, eh.get(8, ts));
    EXPECT_EQ(0, eh.get(9, ts));

    eh.inc(ts, 1);
    EXPECT_EQ(1, eh.get(1, ts));
    EXPECT_EQ(1, eh.get(2, ts));
    EXPECT_EQ(1, eh.get(3, ts));
    EXPECT_EQ(1, eh.get(4, ts));
    EXPECT_EQ(1, eh.get(5, ts));
    EXPECT_EQ(1, eh.get(6, ts));
    EXPECT_EQ(1, eh.get(7, ts));
    EXPECT_EQ(1, eh.get(8, ts));
    EXPECT_EQ(1, eh.get(9, ts));

    // TS = 50
    ts = 50;
    EXPECT_EQ(0, eh.get(1, ts));
    EXPECT_EQ(0, eh.get(2, ts));
    EXPECT_EQ(0, eh.get(3, ts));
    EXPECT_EQ(0, eh.get(4, ts));
    EXPECT_EQ(0, eh.get(5, ts));
    EXPECT_EQ(0, eh.get(6, ts));
    EXPECT_EQ(0, eh.get(7, ts));
    EXPECT_EQ(0, eh.get(8, ts));
    EXPECT_EQ(0, eh.get(9, ts));
}

TEST(ExponentialCountMinSketchTest, Simple) {
    ExponentialCountMinSketch<int> sketch(100, 4, 8);

    uint ts = 0;

    // TS = 0
    ts = 0;
    sketch.inc("aaa", ts, 1);
    sketch.inc(std::string("bbb"), ts, 4);
    sketch.inc("ccc", ts, 8);

    EXPECT_EQ(1,sketch.get(std::string("aaa"), 4, ts));
    EXPECT_EQ(4,sketch.get("bbb", 4, ts));
    EXPECT_EQ(8,sketch.get("ccc", 4, ts));
    EXPECT_EQ(0,sketch.get("ddd", 4, ts));

    // TS = 4
    ts = 4;
    EXPECT_EQ(0,sketch.get("aaa", 2, ts));
    EXPECT_EQ(0,sketch.get("bbb", 2, ts));
    EXPECT_EQ(0,sketch.get(std::string("ccc"), 2, ts));
    EXPECT_EQ(0,sketch.get("ddd", 2, ts));

    EXPECT_EQ(1,sketch.get("aaa", 8, ts));
    EXPECT_EQ(4,sketch.get("bbb", 8, ts));
    EXPECT_EQ(8,sketch.get("ccc", 8, ts));
    EXPECT_EQ(0,sketch.get("ddd", 8, ts));
}

TEST(ExponentialCountMinSketchTest, JustAllocations) {
    for (uint i = 0; i < 100; ++i) {
        ExponentialCountMinSketch<int> sketch(100, 4, 8);
        sketch.inc("aaa", 1000, 1);
        EXPECT_EQ(1,sketch.get("aaa", 8, 1001));
    }
}

}  // namespace
