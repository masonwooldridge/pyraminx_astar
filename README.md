# Pyraminx Search Lab

This project began as an Introduction to Artificial Intelligence course assignment and was later expanded and refined to demonstrate the skills I developed afterward in C++, search algorithms, testing, benchmarking, and WebAssembly.

It is a C++20 solver for a four-layer tetrahedral twisty puzzle. The project compares breadth-first search, A*, and memory-bounded IDA* using admissible heuristics, including a reverse-generated pattern database.

[Try the live WebAssembly demo](https://masonwooldridge.github.io/pyraminx_astar/)

The original 1,300-line prototype is preserved in [`legacy/`](legacy/). The current implementation provides a reusable solver library, command-line interface, automated tests, deterministic benchmarks, and a browser visualizer powered by the same C++ core.

## Features

- Breadth-first search, A*, and IDA* solvers
- Misplaced-sticker and pattern-database heuristics
- Solution replay and verification
- Deterministic benchmarks with node, runtime, and memory estimates
- CMake builds and automated tests on Linux, macOS, and Windows
- WebAssembly visualizer with animated solutions

## Performance

In the fixed-seed benchmark suite, A* with the pattern database expanded 73 median nodes at scramble depth 4, compared with 26,764 for BFS—a 99.7% reduction. IDA* used an estimated 0.8 KiB of search storage at the same depth.

Results were measured on Apple Clang 17 in a release build using five cases per depth. Times are machine-specific, and memory figures estimate stored states and container overhead rather than process RSS. See [`benchmarks/results.md`](benchmarks/results.md) for the full results and methodology.

## Build and use

Requirements: a C++20 compiler and CMake 3.20 or newer.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Solve a deterministic random scramble:

```bash
./build/pyraminx-cli --random 5 --seed 42 --algorithm astar --heuristic pdb
```

Solve an explicit scramble with IDA*:

```bash
./build/pyraminx-cli --scramble "U4 R3' L2 B1" --algorithm idastar
```

Move notation uses `U`, `L`, `R`, or `B` for the tetrahedron axis, followed by a layer number from the tip (`1`) to the opposite outer layer (`4`). A prime denotes a counterclockwise turn.

Run `./build/pyraminx-cli --help` for all solver, heuristic, resource-limit, and display options.

## Benchmarks

```bash
./build/pyraminx-benchmark       # 3 cases at depths 1-4
./build/pyraminx-benchmark 5 5   # 5 cases at depths 1-5
```

The benchmark uses a fixed seed and a ten-second limit per case, then outputs a Markdown results table.

## Browser visualizer

The browser version compiles the C++ solver to WebAssembly with Emscripten.

```bash
emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Release \
  -DPYRAMINX_BUILD_TESTS=OFF -DPYRAMINX_BUILD_BENCHMARKS=OFF
cmake --build build-web --target pyraminx-web
python3 -m http.server 8000 --directory web
```

Open `http://localhost:8000` to enter or generate a scramble, select a solver and heuristic, inspect its metrics, and animate the solution.

## Project structure

```text
include/pyraminx/  Public solver APIs
src/               Solver implementation and CLI
tests/             Invariant, optimality, and verification tests
benchmarks/        Reproducible benchmark suite and results
web/               WebAssembly bindings and browser visualizer
legacy/            Original course-project prototype
```

## License

This project is available under the [MIT License](LICENSE).
