debug-configure:
	cmake -S . -DCMAKE_BUILD_TYPE=Debug -B _debug

debug-build:
	cmake --build _debug

debug-test: debug-build
	_debug/tests/prob_structs_test


release-configure:
	cmake -S . -DCMAKE_BUILD_TYPE=Release -B _release

release-build:
	cmake --build _release

release-test: debug-build
	_release/tests/prob_structs_test
