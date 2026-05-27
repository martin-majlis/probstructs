#include <benchmark/benchmark.h>
#include <string>
#include <vector>
#include "../probstructs/probstructs.h"

using namespace probstructs;

// Pre-generated key pool — avoids measuring string construction inside the timed loop.
static const int KEY_POOL_SIZE = 1024;
static std::vector<std::string> g_keys;

static void InitKeys() {
    if (!g_keys.empty()) return;
    g_keys.reserve(KEY_POOL_SIZE);
    for (int i = 0; i < KEY_POOL_SIZE; ++i) {
        g_keys.push_back("key_" + std::to_string(i));
    }
}

// ---------- CountMinSketch ---------------------------------------------------

class CountMinSketchFixture : public benchmark::Fixture {
public:
    std::unique_ptr<CountMinSketch<int>> sketch;

    void SetUp(const benchmark::State& state) override {
        InitKeys();
        uint32_t width = static_cast<uint32_t>(state.range(0));
        uint8_t  depth = static_cast<uint8_t>(state.range(1));
        sketch = std::make_unique<CountMinSketch<int>>(width, depth);
        for (int i = 0; i < KEY_POOL_SIZE; ++i) {
            sketch->inc(g_keys[i], 1);
        }
    }

    void TearDown(const benchmark::State&) override {
        sketch.reset();
    }
};

BENCHMARK_DEFINE_F(CountMinSketchFixture, Inc)(benchmark::State& state) {
    int64_t i = 0;
    for (auto _ : state) {
        sketch->inc(g_keys[i % KEY_POOL_SIZE], 1);
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_DEFINE_F(CountMinSketchFixture, Get)(benchmark::State& state) {
    int64_t i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sketch->get(g_keys[i % KEY_POOL_SIZE]));
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}

// width, depth
BENCHMARK_REGISTER_F(CountMinSketchFixture, Inc)->Args({100,   3});
BENCHMARK_REGISTER_F(CountMinSketchFixture, Inc)->Args({1000,  5});
BENCHMARK_REGISTER_F(CountMinSketchFixture, Inc)->Args({10000, 7});
BENCHMARK_REGISTER_F(CountMinSketchFixture, Get)->Args({100,   3});
BENCHMARK_REGISTER_F(CountMinSketchFixture, Get)->Args({1000,  5});
BENCHMARK_REGISTER_F(CountMinSketchFixture, Get)->Args({10000, 7});

// ---------- ExponentialHistorgram --------------------------------------------

class ExponentialHistorgramFixture : public benchmark::Fixture {
public:
    std::unique_ptr<ExponentialHistorgram<int>> eh;

    void SetUp(const benchmark::State& state) override {
        uint32_t window = static_cast<uint32_t>(state.range(0));
        eh = std::make_unique<ExponentialHistorgram<int>>(window);
        for (uint32_t t = 0; t < 64; ++t) {
            eh->inc(t, 1);
        }
    }

    void TearDown(const benchmark::State&) override {
        eh.reset();
    }
};

BENCHMARK_DEFINE_F(ExponentialHistorgramFixture, Inc)(benchmark::State& state) {
    uint32_t tick = 1000;
    for (auto _ : state) {
        eh->inc(tick, 1);
        ++tick;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_DEFINE_F(ExponentialHistorgramFixture, Get)(benchmark::State& state) {
    uint32_t window = static_cast<uint32_t>(state.range(0));
    uint32_t tick   = 1000;
    for (auto _ : state) {
        benchmark::DoNotOptimize(eh->get(window / 2, tick));
        ++tick;
    }
    state.SetItemsProcessed(state.iterations());
}

// window
BENCHMARK_REGISTER_F(ExponentialHistorgramFixture, Inc)->Arg(64);
BENCHMARK_REGISTER_F(ExponentialHistorgramFixture, Inc)->Arg(1024);
BENCHMARK_REGISTER_F(ExponentialHistorgramFixture, Inc)->Arg(16384);
BENCHMARK_REGISTER_F(ExponentialHistorgramFixture, Get)->Arg(64);
BENCHMARK_REGISTER_F(ExponentialHistorgramFixture, Get)->Arg(1024);
BENCHMARK_REGISTER_F(ExponentialHistorgramFixture, Get)->Arg(16384);

// ---------- ExponentialCountMinSketch ----------------------------------------

class ECMSketchFixture : public benchmark::Fixture {
public:
    std::unique_ptr<ExponentialCountMinSketch<int>> sketch;
    uint32_t window_;

    void SetUp(const benchmark::State& state) override {
        InitKeys();
        uint32_t width = static_cast<uint32_t>(state.range(0));
        uint8_t  depth = static_cast<uint8_t>(state.range(1));
        window_        = static_cast<uint32_t>(state.range(2));
        sketch = std::make_unique<ExponentialCountMinSketch<int>>(width, depth, window_);
        for (int i = 0; i < KEY_POOL_SIZE; ++i) {
            sketch->inc(g_keys[i], static_cast<uint32_t>(i), 1);
        }
    }

    void TearDown(const benchmark::State&) override {
        sketch.reset();
    }
};

BENCHMARK_DEFINE_F(ECMSketchFixture, Inc)(benchmark::State& state) {
    uint32_t tick = KEY_POOL_SIZE + 1;
    int64_t  i    = 0;
    for (auto _ : state) {
        sketch->inc(g_keys[i % KEY_POOL_SIZE], tick, 1);
        ++tick;
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_DEFINE_F(ECMSketchFixture, Get)(benchmark::State& state) {
    uint32_t tick = KEY_POOL_SIZE + 1;
    int64_t  i    = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sketch->get(g_keys[i % KEY_POOL_SIZE], window_ / 2, tick));
        ++tick;
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}

// width, depth, window
BENCHMARK_REGISTER_F(ECMSketchFixture, Inc)->Args({100,  3,   64});
BENCHMARK_REGISTER_F(ECMSketchFixture, Inc)->Args({1000, 5, 1024});
BENCHMARK_REGISTER_F(ECMSketchFixture, Inc)->Args({5000, 7, 4096});
BENCHMARK_REGISTER_F(ECMSketchFixture, Get)->Args({100,  3,   64});
BENCHMARK_REGISTER_F(ECMSketchFixture, Get)->Args({1000, 5, 1024});
BENCHMARK_REGISTER_F(ECMSketchFixture, Get)->Args({5000, 7, 4096});
