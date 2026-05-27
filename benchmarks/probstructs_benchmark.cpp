#include <benchmark/benchmark.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "../probstructs/probstructs.h"

using namespace probstructs;

// ============================================================================
// Shared test data — generated once at process start
//
// Three key-domain sizes mirror typical real workloads:
//   POOL_SMALL  ( 1 K) — high collision rate, fits fully in L1/L2 cache
//   POOL_MEDIUM (10 K) — moderate collisions, L2/L3 resident
//   POOL_LARGE (100 K) — low collision rate, exercises cache pressure
//
// Two access distributions per pool:
//   Uniform — every key equally likely
//   Zipf(s=1) — top 1 % of keys receive ~50 % of traffic; realistic for
//               web requests, event streams, user activity logs, etc.
// ============================================================================

static const int POOL_SMALL  =   1'000;
static const int POOL_MEDIUM =  10'000;
static const int POOL_LARGE  = 100'000;
static const int SEQ_LEN     =  1'000'000;

static std::vector<std::string> g_keys_small;
static std::vector<std::string> g_keys_medium;
static std::vector<std::string> g_keys_large;

static std::vector<int> g_uniform_small,  g_uniform_medium,  g_uniform_large;
static std::vector<int> g_zipf_small,     g_zipf_medium,     g_zipf_large;

static void MakeKeys(std::vector<std::string>& v, int n) {
    if (!v.empty()) return;
    v.reserve(n);
    for (int i = 0; i < n; ++i) v.push_back("key_" + std::to_string(i));
}

static std::vector<int> MakeUniformSeq(int pool, int len) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, pool - 1);
    std::vector<int> seq(len);
    for (auto& v : seq) v = dist(rng);
    return seq;
}

// Discrete Zipf sampler via precomputed CDF + binary search.
static std::vector<int> MakeZipfSeq(int pool, int len) {
    std::vector<double> cdf(pool);
    double total = 0.0;
    for (int k = 1; k <= pool; ++k) { total += 1.0 / k; cdf[k - 1] = total; }
    for (auto& v : cdf) v /= total;

    std::mt19937 rng(43);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::vector<int> seq(len);
    for (auto& idx : seq) {
        double r = dist(rng);
        idx = static_cast<int>(std::lower_bound(cdf.begin(), cdf.end(), r) - cdf.begin());
        if (idx >= pool) idx = pool - 1;
    }
    return seq;
}

static void InitGlobals() {
    static bool done = false;
    if (done) return;
    done = true;
    MakeKeys(g_keys_small,  POOL_SMALL);
    MakeKeys(g_keys_medium, POOL_MEDIUM);
    MakeKeys(g_keys_large,  POOL_LARGE);
    g_uniform_small  = MakeUniformSeq(POOL_SMALL,  SEQ_LEN);
    g_uniform_medium = MakeUniformSeq(POOL_MEDIUM, SEQ_LEN);
    g_uniform_large  = MakeUniformSeq(POOL_LARGE,  SEQ_LEN);
    g_zipf_small     = MakeZipfSeq(POOL_SMALL,  SEQ_LEN);
    g_zipf_medium    = MakeZipfSeq(POOL_MEDIUM, SEQ_LEN);
    g_zipf_large     = MakeZipfSeq(POOL_LARGE,  SEQ_LEN);
}

// pool_id 0/1/2 → small/medium/large
static const std::vector<std::string>& KeyPool(int id) {
    if (id == 0) return g_keys_small;
    if (id == 1) return g_keys_medium;
    return g_keys_large;
}

// dist_id 0=uniform, 1=zipf
static const std::vector<int>& AccessSeq(int pool_id, int dist_id) {
    if (dist_id == 1) {
        if (pool_id == 0) return g_zipf_small;
        if (pool_id == 1) return g_zipf_medium;
        return g_zipf_large;
    }
    if (pool_id == 0) return g_uniform_small;
    if (pool_id == 1) return g_uniform_medium;
    return g_uniform_large;
}

// ============================================================================
// CountMinSketch
//
// Parameters (via state.range):
//   [0] width    — sketch width (controls accuracy vs memory)
//   [1] depth    — number of hash functions (controls accuracy vs speed)
//   [2] pool_id  — key domain size: 0=1K, 1=10K, 2=100K
//   [3] dist_id  — access distribution: 0=uniform, 1=zipf
//
// With a uniform distribution, CMS latency is dominated purely by the hash +
// memory-write pattern.  Zipf makes the hot bucket lines stay warm in cache,
// which can show a modest improvement.  What matters more is whether the full
// sketch (width * depth * sizeof(int) bytes) fits in L1 (≤ 32 KB), L2, or
// spills to L3.
// ============================================================================

class CMSFixture : public benchmark::Fixture {
public:
    std::unique_ptr<CountMinSketch<int>> sketch;
    const std::vector<std::string>* keys = nullptr;
    const std::vector<int>*         seq  = nullptr;

    void SetUp(const benchmark::State& state) override {
        InitGlobals();
        keys   = &KeyPool(state.range(2));
        seq    = &AccessSeq(state.range(2), state.range(3));
        sketch = std::make_unique<CountMinSketch<int>>(
                     static_cast<uint32_t>(state.range(0)),
                     static_cast<uint8_t> (state.range(1)));
        for (const auto& k : *keys) sketch->inc(k, 1);
    }

    void TearDown(const benchmark::State&) override { sketch.reset(); }
};

BENCHMARK_DEFINE_F(CMSFixture, Inc)(benchmark::State& state) {
    int64_t i = 0;
    for (auto _ : state) {
        sketch->inc((*keys)[(*seq)[i % SEQ_LEN]], 1);
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_DEFINE_F(CMSFixture, Get)(benchmark::State& state) {
    int64_t i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(sketch->get((*keys)[(*seq)[i % SEQ_LEN]]));
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}

// 80 % inc / 20 % get — typical read-light monitoring workload
BENCHMARK_DEFINE_F(CMSFixture, Mixed)(benchmark::State& state) {
    int64_t i = 0;
    for (auto _ : state) {
        const std::string& k = (*keys)[(*seq)[i % SEQ_LEN]];
        if (i % 5 == 0) benchmark::DoNotOptimize(sketch->get(k));
        else             sketch->inc(k, 1);
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}

// -- Inc: uniform, three key domains, two sketch sizes ----------------------
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({ 500, 3, 0, 0})->Name("CMS/Inc/w500d3/uniform/1K");
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({2000, 5, 0, 0})->Name("CMS/Inc/w2000d5/uniform/1K");
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({ 500, 3, 1, 0})->Name("CMS/Inc/w500d3/uniform/10K");
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({2000, 5, 1, 0})->Name("CMS/Inc/w2000d5/uniform/10K");
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({ 500, 3, 2, 0})->Name("CMS/Inc/w500d3/uniform/100K");
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({2000, 5, 2, 0})->Name("CMS/Inc/w2000d5/uniform/100K");
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({5000, 7, 2, 0})->Name("CMS/Inc/w5000d7/uniform/100K");
// -- Inc: Zipf — same sketch sizes, shows hot-key cache effect ---------------
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({ 500, 3, 0, 1})->Name("CMS/Inc/w500d3/zipf/1K");
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({ 500, 3, 1, 1})->Name("CMS/Inc/w500d3/zipf/10K");
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({2000, 5, 1, 1})->Name("CMS/Inc/w2000d5/zipf/10K");
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({ 500, 3, 2, 1})->Name("CMS/Inc/w500d3/zipf/100K");
BENCHMARK_REGISTER_F(CMSFixture, Inc)->Args({2000, 5, 2, 1})->Name("CMS/Inc/w2000d5/zipf/100K");
// -- Get ----------------------------------------------------------------------
BENCHMARK_REGISTER_F(CMSFixture, Get)->Args({ 500, 3, 1, 0})->Name("CMS/Get/w500d3/uniform/10K");
BENCHMARK_REGISTER_F(CMSFixture, Get)->Args({2000, 5, 1, 0})->Name("CMS/Get/w2000d5/uniform/10K");
BENCHMARK_REGISTER_F(CMSFixture, Get)->Args({ 500, 3, 1, 1})->Name("CMS/Get/w500d3/zipf/10K");
BENCHMARK_REGISTER_F(CMSFixture, Get)->Args({2000, 5, 2, 1})->Name("CMS/Get/w2000d5/zipf/100K");
// -- Mixed --------------------------------------------------------------------
BENCHMARK_REGISTER_F(CMSFixture, Mixed)->Args({2000, 5, 1, 0})->Name("CMS/Mixed/w2000d5/uniform/10K");
BENCHMARK_REGISTER_F(CMSFixture, Mixed)->Args({2000, 5, 1, 1})->Name("CMS/Mixed/w2000d5/zipf/10K");
BENCHMARK_REGISTER_F(CMSFixture, Mixed)->Args({2000, 5, 2, 1})->Name("CMS/Mixed/w2000d5/zipf/100K");

// ============================================================================
// ExponentialHistorgram
//
// Parameters (via state.range):
//   [0] window     — sliding window size
//   [1] tick_step  — tick increment per operation
//
// tick_step is the key variable: how far the stream advances between calls.
//   step = 1           → steady stream, minimal bucket reshuffling (hot path)
//   step = window / 4  → bursty, several buckets shift each call
//   step = window / 2  → heavy churn, most stored data expires on every call
//
// This directly reveals the cost of the internal bucket-cascade algorithm
// across realistic operating points.
// ============================================================================

class EHFixture : public benchmark::Fixture {
public:
    std::unique_ptr<ExponentialHistorgram<int>> eh;

    void SetUp(const benchmark::State& state) override {
        eh = std::make_unique<ExponentialHistorgram<int>>(
                 static_cast<uint32_t>(state.range(0)));
        for (uint32_t t = 0; t < 128; ++t) eh->inc(t, 1);
    }

    void TearDown(const benchmark::State&) override { eh.reset(); }
};

BENCHMARK_DEFINE_F(EHFixture, Inc)(benchmark::State& state) {
    uint32_t step = static_cast<uint32_t>(state.range(1));
    uint32_t tick = 2000;
    for (auto _ : state) {
        eh->inc(tick, 1);
        tick += step;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_DEFINE_F(EHFixture, Get)(benchmark::State& state) {
    uint32_t window = static_cast<uint32_t>(state.range(0));
    uint32_t step   = static_cast<uint32_t>(state.range(1));
    uint32_t tick   = 2000;
    for (auto _ : state) {
        benchmark::DoNotOptimize(eh->get(window / 2, tick));
        tick += step;
    }
    state.SetItemsProcessed(state.iterations());
}

// window, tick_step
// Steady stream (tick_step = 1)
BENCHMARK_REGISTER_F(EHFixture, Inc)->Args({  256,    1})->Name("EH/Inc/w256/steady");
BENCHMARK_REGISTER_F(EHFixture, Inc)->Args({ 1024,    1})->Name("EH/Inc/w1024/steady");
BENCHMARK_REGISTER_F(EHFixture, Inc)->Args({16384,    1})->Name("EH/Inc/w16384/steady");
// Bursty stream (tick_step = window / 4)
BENCHMARK_REGISTER_F(EHFixture, Inc)->Args({  256,   64})->Name("EH/Inc/w256/bursty");
BENCHMARK_REGISTER_F(EHFixture, Inc)->Args({ 1024,  256})->Name("EH/Inc/w1024/bursty");
BENCHMARK_REGISTER_F(EHFixture, Inc)->Args({16384, 4096})->Name("EH/Inc/w16384/bursty");
// High-churn (tick_step = window / 2 → most data expires each call)
BENCHMARK_REGISTER_F(EHFixture, Inc)->Args({  256,  128})->Name("EH/Inc/w256/expiry");
BENCHMARK_REGISTER_F(EHFixture, Inc)->Args({ 1024,  512})->Name("EH/Inc/w1024/expiry");
BENCHMARK_REGISTER_F(EHFixture, Inc)->Args({16384, 8192})->Name("EH/Inc/w16384/expiry");

BENCHMARK_REGISTER_F(EHFixture, Get)->Args({  256,    1})->Name("EH/Get/w256/steady");
BENCHMARK_REGISTER_F(EHFixture, Get)->Args({ 1024,    1})->Name("EH/Get/w1024/steady");
BENCHMARK_REGISTER_F(EHFixture, Get)->Args({16384,    1})->Name("EH/Get/w16384/steady");
BENCHMARK_REGISTER_F(EHFixture, Get)->Args({ 1024,  256})->Name("EH/Get/w1024/bursty");
BENCHMARK_REGISTER_F(EHFixture, Get)->Args({16384, 4096})->Name("EH/Get/w16384/bursty");
BENCHMARK_REGISTER_F(EHFixture, Get)->Args({ 1024,  512})->Name("EH/Get/w1024/expiry");

// ============================================================================
// ExponentialCountMinSketch
//
// Parameters (via state.range):
//   [0] width     — sketch width
//   [1] depth     — number of hash functions
//   [2] window    — sliding window size
//   [3] tick_step — tick increment (1=steady, larger=bursty)
//   [4] pool_id   — key domain: 0=1K, 1=10K, 2=100K
//   [5] dist_id   — access pattern: 0=uniform, 1=zipf
// ============================================================================

class ECMFixture : public benchmark::Fixture {
public:
    std::unique_ptr<ExponentialCountMinSketch<int>> sketch;
    const std::vector<std::string>* keys = nullptr;
    const std::vector<int>*         seq  = nullptr;
    uint32_t window_ = 0;
    uint32_t step_   = 1;

    void SetUp(const benchmark::State& state) override {
        InitGlobals();
        window_ = static_cast<uint32_t>(state.range(2));
        step_   = static_cast<uint32_t>(state.range(3));
        keys    = &KeyPool(state.range(4));
        seq     = &AccessSeq(state.range(4), state.range(5));
        sketch  = std::make_unique<ExponentialCountMinSketch<int>>(
                      static_cast<uint32_t>(state.range(0)),
                      static_cast<uint8_t> (state.range(1)),
                      window_);
        for (size_t i = 0; i < keys->size(); ++i)
            sketch->inc((*keys)[i], static_cast<uint32_t>(i), 1);
    }

    void TearDown(const benchmark::State&) override { sketch.reset(); }
};

BENCHMARK_DEFINE_F(ECMFixture, Inc)(benchmark::State& state) {
    uint32_t tick = static_cast<uint32_t>(keys->size()) + 1;
    int64_t  i    = 0;
    for (auto _ : state) {
        sketch->inc((*keys)[(*seq)[i % SEQ_LEN]], tick, 1);
        tick += step_;
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_DEFINE_F(ECMFixture, Get)(benchmark::State& state) {
    uint32_t tick = static_cast<uint32_t>(keys->size()) + 1;
    int64_t  i    = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(
            sketch->get((*keys)[(*seq)[i % SEQ_LEN]], window_ / 2, tick));
        tick += step_;
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}

// 80 % inc / 20 % get
BENCHMARK_DEFINE_F(ECMFixture, Mixed)(benchmark::State& state) {
    uint32_t tick = static_cast<uint32_t>(keys->size()) + 1;
    int64_t  i    = 0;
    for (auto _ : state) {
        const std::string& k = (*keys)[(*seq)[i % SEQ_LEN]];
        if (i % 5 == 0) benchmark::DoNotOptimize(sketch->get(k, window_ / 2, tick));
        else             sketch->inc(k, tick, 1);
        tick += step_;
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}

// width, depth, window, tick_step, pool_id, dist_id
// -- Steady stream, uniform ---------------------------------------------------
BENCHMARK_REGISTER_F(ECMFixture, Inc)
    ->Args({ 500, 3,  256, 1, 0, 0})->Name("ECM/Inc/w500d3win256/steady/uniform/1K");
BENCHMARK_REGISTER_F(ECMFixture, Inc)
    ->Args({ 500, 3, 1024, 1, 1, 0})->Name("ECM/Inc/w500d3win1024/steady/uniform/10K");
BENCHMARK_REGISTER_F(ECMFixture, Inc)
    ->Args({2000, 5, 1024, 1, 1, 0})->Name("ECM/Inc/w2000d5win1024/steady/uniform/10K");
BENCHMARK_REGISTER_F(ECMFixture, Inc)
    ->Args({2000, 5, 4096, 1, 2, 0})->Name("ECM/Inc/w2000d5win4096/steady/uniform/100K");
// -- Steady stream, Zipf — hot keys stay warm in EH buckets ------------------
BENCHMARK_REGISTER_F(ECMFixture, Inc)
    ->Args({ 500, 3, 1024, 1, 1, 1})->Name("ECM/Inc/w500d3win1024/steady/zipf/10K");
BENCHMARK_REGISTER_F(ECMFixture, Inc)
    ->Args({2000, 5, 1024, 1, 1, 1})->Name("ECM/Inc/w2000d5win1024/steady/zipf/10K");
BENCHMARK_REGISTER_F(ECMFixture, Inc)
    ->Args({2000, 5, 4096, 1, 2, 1})->Name("ECM/Inc/w2000d5win4096/steady/zipf/100K");
// -- Bursty stream — reveals sliding-window maintenance overhead --------------
BENCHMARK_REGISTER_F(ECMFixture, Inc)
    ->Args({ 500, 3, 1024, 256, 1, 0})->Name("ECM/Inc/w500d3win1024/bursty/uniform/10K");
BENCHMARK_REGISTER_F(ECMFixture, Inc)
    ->Args({2000, 5, 1024, 256, 1, 0})->Name("ECM/Inc/w2000d5win1024/bursty/uniform/10K");
BENCHMARK_REGISTER_F(ECMFixture, Inc)
    ->Args({2000, 5, 1024, 256, 1, 1})->Name("ECM/Inc/w2000d5win1024/bursty/zipf/10K");
// -- Get ----------------------------------------------------------------------
BENCHMARK_REGISTER_F(ECMFixture, Get)
    ->Args({ 500, 3, 1024, 1, 1, 0})->Name("ECM/Get/w500d3win1024/steady/uniform/10K");
BENCHMARK_REGISTER_F(ECMFixture, Get)
    ->Args({2000, 5, 1024, 1, 1, 1})->Name("ECM/Get/w2000d5win1024/steady/zipf/10K");
BENCHMARK_REGISTER_F(ECMFixture, Get)
    ->Args({ 500, 3, 1024, 256, 1, 0})->Name("ECM/Get/w500d3win1024/bursty/uniform/10K");
BENCHMARK_REGISTER_F(ECMFixture, Get)
    ->Args({2000, 5, 4096, 1, 2, 1})->Name("ECM/Get/w2000d5win4096/steady/zipf/100K");
// -- Mixed --------------------------------------------------------------------
BENCHMARK_REGISTER_F(ECMFixture, Mixed)
    ->Args({ 500, 3, 1024,   1, 1, 0})->Name("ECM/Mixed/w500d3win1024/steady/uniform/10K");
BENCHMARK_REGISTER_F(ECMFixture, Mixed)
    ->Args({2000, 5, 1024,   1, 1, 1})->Name("ECM/Mixed/w2000d5win1024/steady/zipf/10K");
BENCHMARK_REGISTER_F(ECMFixture, Mixed)
    ->Args({ 500, 3, 1024, 256, 1, 0})->Name("ECM/Mixed/w500d3win1024/bursty/uniform/10K");

// ============================================================================
// End-to-end scenarios
//
// These simulate realistic workloads rather than isolating a single operation.
// ============================================================================

// Scenario: frequency estimation over a large, Zipf-distributed domain.
// Models a system counting how often each event (URL, user, product) appears
// in a stream.  90 % inserts, 10 % point queries (e.g., "is this key hot?").
static void BM_FrequencyEstimation(benchmark::State& state) {
    InitGlobals();
    CountMinSketch<int> sketch(5000, 5);
    for (int i = 0; i < SEQ_LEN; ++i)
        sketch.inc(g_keys_large[g_zipf_large[i]], 1);

    int64_t i = 0;
    for (auto _ : state) {
        sketch.inc(g_keys_large[g_zipf_large[i % SEQ_LEN]], 1);
        if (i % 10 == 0)
            benchmark::DoNotOptimize(sketch.get(g_keys_large[g_zipf_large[i % SEQ_LEN]]));
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_FrequencyEstimation)->Name("Scenario/FrequencyEstimation/zipf/100K");

// Scenario: heavy-hitter detection in a sliding window.
// Simulates a monitoring system tracking per-key frequency in the last N events
// (e.g., detecting DDoS sources, trending topics, or slow queries).
// Every 10 inserts, queries the current key and flags it as a heavy hitter
// if its count exceeds 1 % of the window size.
static void BM_HeavyHitterDetection(benchmark::State& state) {
    InitGlobals();
    const uint32_t window    = 2048;
    const int      threshold = static_cast<int>(window * 0.01);
    ExponentialCountMinSketch<int> sketch(1000, 5, window);
    for (size_t i = 0; i < g_keys_medium.size(); ++i)
        sketch.inc(g_keys_medium[i], static_cast<uint32_t>(i), 1);

    uint32_t tick         = static_cast<uint32_t>(g_keys_medium.size()) + 1;
    int64_t  i            = 0;
    int64_t  heavy_hitters = 0;
    for (auto _ : state) {
        const std::string& k = g_keys_medium[g_zipf_medium[i % SEQ_LEN]];
        sketch.inc(k, tick, 1);
        if (i % 10 == 0) {
            int cnt = sketch.get(k, window, tick);
            if (cnt > threshold) ++heavy_hitters;
        }
        ++tick;
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
    state.counters["heavy_hitters_pct"] =
        benchmark::Counter(static_cast<double>(heavy_hitters) /
                           static_cast<double>(state.iterations()) * 100.0);
}
BENCHMARK(BM_HeavyHitterDetection)->Name("Scenario/HeavyHitterDetection/zipf/10K");

// Scenario: sliding-window rate limiter.
// Each tick represents one second.  For every request (key = client ID),
// the system checks whether the client has exceeded R requests in the last W
// seconds.  Mirrors rate-limiting and quota enforcement use cases.
static void BM_RateLimiter(benchmark::State& state) {
    InitGlobals();
    const uint32_t window        = 60;   // 60-second sliding window
    const int      rate_limit    = 10;   // max 10 requests per window
    ExponentialCountMinSketch<int> sketch(2000, 5, window);
    for (size_t i = 0; i < g_keys_medium.size(); ++i)
        sketch.inc(g_keys_medium[i], static_cast<uint32_t>(i % window), 1);

    uint32_t tick      = static_cast<uint32_t>(g_keys_medium.size()) + 1;
    int64_t  i         = 0;
    int64_t  throttled = 0;
    for (auto _ : state) {
        const std::string& k = g_keys_medium[g_zipf_medium[i % SEQ_LEN]];
        int rate = sketch.get(k, window, tick);
        if (rate < rate_limit) {
            sketch.inc(k, tick, 1);
        } else {
            ++throttled;
        }
        if (i % 10 == 0) ++tick;   // advance clock every 10 requests
        ++i;
    }
    state.SetItemsProcessed(state.iterations());
    state.counters["throttled_pct"] =
        benchmark::Counter(static_cast<double>(throttled) /
                           static_cast<double>(state.iterations()) * 100.0);
}
BENCHMARK(BM_RateLimiter)->Name("Scenario/RateLimiter/zipf/10K");
