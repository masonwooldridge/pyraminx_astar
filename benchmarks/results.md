# Benchmark results

These results were generated on August 25, 2026 with Apple Clang 17, `-O2`, five fixed-seed scrambles per requested depth, a two-million-node limit, and a ten-second wall-clock limit per solve:

```bash
./build/pyraminx-benchmark 5 5
```

The generator uses `std::mt19937(0xC0FFEE)` and rejects consecutive moves of the same layer. Each solver receives the same states. Solution length is validated by replay; BFS supplies the tractable-depth optimality baseline.

| Scramble depth | Solver | Heuristic | Solved | Median solution | Median expanded | Median time (ms) | Est. peak (KiB) | Heuristic accuracy |
|---:|---|---|---:|---:|---:|---:|---:|---:|
| 1 | bfs | zero | 5/5 | 1 | 10 | 0.047 | 76.9 | 0% |
| 1 | astar | pdb | 5/5 | 1 | 2 | 0.010 | 10.2 | 100% |
| 1 | idastar | pdb | 5/5 | 1 | 1 | 0.002 | 0.3 | 100% |
| 2 | bfs | zero | 5/5 | 2 | 196 | 1.369 | 1,208.6 | 0% |
| 2 | astar | pdb | 5/5 | 2 | 6 | 0.098 | 46.1 | 50% |
| 2 | idastar | pdb | 5/5 | 2 | 7 | 0.061 | 0.5 | 50% |
| 3 | bfs | zero | 5/5 | 3 | 5,006 | 36.363 | 28,733.6 | 0% |
| 3 | astar | pdb | 5/5 | 3 | 11 | 0.108 | 86.5 | 67% |
| 3 | idastar | pdb | 5/5 | 3 | 20 | 0.149 | 0.6 | 67% |
| 4 | bfs | zero | 5/5 | 4 | 26,764 | 448.510 | 152,194.5 | 0% |
| 4 | astar | pdb | 5/5 | 4 | 73 | 0.689 | 548.7 | 50% |
| 4 | idastar | pdb | 5/5 | 4 | 140 | 1.158 | 0.8 | 50% |
| 5 | bfs | zero | 0/5 | — | — | — | — | — |
| 5 | astar | pdb | 5/5 | 5 | 882 | 9.598 | 6,461.5 | 40% |
| 5 | idastar | pdb | 5/5 | 5 | 6,721 | 52.291 | 0.9 | 40% |

“Estimated peak” models stored search states and approximate hash-container overhead. It should be used for within-project comparisons, not as a claim about process RSS. Timing varies by machine and build.
