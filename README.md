# Random Dungeon Generator — C++ Port

A C++ port/modernization of **drow's Random Dungeon Generator** from [donjon.bin.sh](https://donjon.bin.sh/).

The repository explores translating the original Perl dungeon-generation algorithm into native C++ with a reusable header/API, CMake build, tests and a small command-line renderer.

## What is here

- `rdg.h` — C++ dungeon-generation implementation/API
- `main.cpp` — minimal CLI example that generates and prints a dungeon
- `tests/` — automated verification
- `dungeon.pl` — original/reference implementation
- `vstd` / `vstd.h` — supporting utility dependency

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Provenance and license

The included `dungeon.pl` identifies the original implementation as **Random Dungeon Generator by drow** from `donjon.bin.sh` and states that it is provided under the **Creative Commons Attribution-NonCommercial 3.0 Unported License**.

This C++ implementation is a derivative port of that algorithm and should not be interpreted as an independently invented dungeon-generation algorithm. The original attribution is retained deliberately.

## Status

Experimental/educational native-code port. A separate Kotlin/JVM port is available in the `dungeon` repository.
