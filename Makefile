# Locate cmake — checks PATH first, then common Homebrew locations.
CMAKE := $(or \
	$(shell which cmake 2>/dev/null), \
	$(wildcard /opt/homebrew/bin/cmake), \
	$(wildcard /usr/local/bin/cmake))

# Fail early with a helpful message if cmake is nowhere to be found.
ifeq ($(CMAKE),)
  $(error cmake not found. Install it with: brew install cmake  (macOS) \
  or see https://apt.kitware.com/ (Linux))
endif

# sudo aptitude install doxygen graphviz
# pip3 install breathe sphinx_rtd_theme

_debug: debug-configure

debug-configure:
	$(CMAKE) -S . -DCMAKE_BUILD_TYPE=Debug -B _debug

debug-build: _debug
	$(CMAKE) --build _debug

debug-test: debug-build
	_debug/tests/probstructs_test

debug-valgrind: debug-build
	valgrind --leak-check=yes -v _debug/tests/prob_structs_test


_release: release-configure

release-configure:
	$(CMAKE) -S . -DCMAKE_BUILD_TYPE=Release -B _release

release-build: _release
	$(CMAKE) --build _release

release-test: release-build
	_release/tests/probstructs_test


clean:
	rm -rf _debug _release _bench


_bench: bench-configure

bench-configure:
	$(CMAKE) -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON -B _bench

bench-build: _bench
	$(CMAKE) --build _bench --target probstructs_benchmark

bench-run: bench-build
	./scripts/bench_run.sh _bench/benchmarks/probstructs_benchmark

bench-compare:
	./scripts/bench_compare.sh