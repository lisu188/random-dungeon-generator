# Random dungeon generator

The header-only `rdg` target requires C++26 with extensions disabled and no fallback, and CMake 4.0.2 or newer.
CI provisions CMake 4.3.1 and validates GCC 15 and Clang 20 on Ubuntu 26.04, and LLVM 20.1.8 clang-cl with Ninja
and the Visual Studio 2022 SDK, runtime, and standard library on Windows 2022. Configuration probes pack indexing,
deleted-function reasons, coroutines, `std::expected`, and `std::move_only_function`.

```sh
cmake -S . -B build-cxx26-gcc15 -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++-15
cmake --build build-cxx26-gcc15 --parallel
ctest --test-dir build-cxx26-gcc15 --output-on-failure
GCOV=gcov-15 bash scripts/check_coverage.sh build-cxx26-gcc15 80
```

Use separate build directories when changing compilers. Debug builds with a GNU-style frontend retain coverage
instrumentation; clang-cl uses the Visual Studio flags and does not receive GCC coverage flags. The existing RDG
eligible-line gate remains 80%. The game has its own validation and coverage requirements.

Enum values, option defaults, room/door/stair order, and random-number consumption are compatibility contracts.
String-to-enum, enum-to-string, and metadata-key helpers now support constant evaluation. A generation uses exactly
two draws from the supplied `std::mt19937` to initialize its internal generator; the supplied generator remains
caller-owned.

CI compares complete public dungeon output and RNG state against the pull request base, across all four dungeon
layouts and eight fixed seeds. The comparison includes every cell type, room id, label, room/door/stair field and
ordering, the complete external RNG state, and sixteen subsequent draws. Run the same comparison with:

```sh
python scripts/compare_seeded_output.py --compiler g++-15 --baseline-ref <base-commit>
```

On Windows, use `--compiler clang-cl --frontend msvc` in a Visual Studio development environment. The script keeps
snapshots, compile logs, baseline headers, and a manifest in `parity-artifacts/`, including on failure. This proves
generator compatibility for the selected scenarios; game integration and runtime acceptance require separate
validation.
