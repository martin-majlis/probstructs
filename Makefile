_debug: debug-configure

debug-configure:
	cmake -S . -DCMAKE_BUILD_TYPE=Debug -B _debug

debug-build: _debug
	cmake --build _debug

debug-test: debug-build
	_debug/tests/prob_structs_test


_release: release-configure

release-configure:
	cmake -S . -DCMAKE_BUILD_TYPE=Release -B _release

release-build: _release
	cmake --build _release

release-test: release-build
	_release/tests/prob_structs_test


clean:
	rm -rf _debug _release