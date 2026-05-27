# You need latest cmake to make it work
# https://apt.kitware.com/

# sudo aptitude install doxygen graphviz
# pip3 install breathe sphinx_rtd_theme

_debug: debug-configure

debug-configure:
	cmake -S . -DCMAKE_BUILD_TYPE=Debug -B _debug

debug-build: _debug
	cmake --build _debug

debug-test: debug-build
	_debug/tests/probstructs_test

debug-valgrind: debug-build
	valgrind --leak-check=yes -v _debug/tests/prob_structs_test


_release: release-configure

release-configure:
	cmake -S . -DCMAKE_BUILD_TYPE=Release -B _release

release-build: _release
	cmake --build _release

release-test: release-build
	_release/tests/probstructs_test


clean:
	rm -rf _debug _release _bench


_bench: bench-configure

bench-configure:
	cmake -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON -B _bench

bench-build: _bench
	cmake --build _bench --target probstructs_benchmark

bench-run: bench-build
	./scripts/bench_run.sh _bench/benchmarks/probstructs_benchmark

bench-compare:
	./scripts/bench_compare.sh