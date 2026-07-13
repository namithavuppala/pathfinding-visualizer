# Pathfinding Visualizer (C++)

A terminal pathfinding visualizer implementing four classic graph-search
algorithms — **BFS, DFS, Dijkstra, and A\*** — in modern C++ (C++17), with the
**binary min-heap priority queue that powers Dijkstra and A\* implemented from
scratch** (a templated `MinHeap<T, Compare>`, no `std::priority_queue`).

Run it and watch each algorithm search a randomly-generated maze cell by cell,
then trace the shortest path — or run all four on the same maze and compare how
many nodes each one explores.

## The DSA, and why each structure

| Algorithm | Data structure | Shortest path? | Idea |
|-----------|----------------|----------------|------|
| **BFS**   | Queue (FIFO)   | ✅ (unweighted) | Explore in rings of equal distance |
| **DFS**   | Stack (LIFO)   | ❌              | Dive deep down one branch first |
| **Dijkstra** | Custom min-heap | ✅ (weighted) | Always expand the closest node |
| **A\***   | Min-heap on `f = g + h` | ✅ (weighted) | Dijkstra guided toward the goal |

The templated **`MinHeap`** in [`include/MinHeap.hpp`](include/MinHeap.hpp) is the
heart of the project: a binary heap giving `O(log n)` push/pop. Dijkstra and A\*
use **lazy deletion** — a node can be pushed again when a shorter route is found,
and stale copies are skipped on pop — so there's no need for a decrease-key
operation.

## Sample output (`./pathfinder all`)

```
  BFS         visited: 537   path: 35
  DFS         visited: 423   path: 339
  Dijkstra    visited: 537   path: 35
  A*          visited: 86    path: 35
```

Same maze, same optimal length (35) for BFS / Dijkstra / A\* — but **A\* reaches
it after exploring only 86 cells versus 537**, because the admissible Manhattan
heuristic steers it toward the goal. DFS finds *a* path, but a far longer one
(339) — a concrete reminder that DFS does not minimise distance.

## Complexity

Let `V` = cells, `E` = edges (≤ 4·V on a 4-connected grid).

| Algorithm | Time | Space |
|-----------|------|-------|
| BFS / DFS | `O(V + E)` | `O(V)` |
| Dijkstra  | `O(E log V)` | `O(V)` |
| A\*       | `O(E log V)` worst case; explores far fewer nodes with a good heuristic | `O(V)` |

## Build & run

Requires a C++17 compiler (`g++` or `clang++`). No external dependencies.

```bash
make                       # builds ./pathfinder

./pathfinder all           # run all four, print the comparison table
./pathfinder astar --animate   # watch A* search the grid live (ANSI terminal)
./pathfinder bfs           # BFS: final grid + stats
./pathfinder dijkstra      # etc.
```

Legend in the animated/final grid: **green** start · **red** finish ·
**gray** wall · **blue** visited · **yellow** shortest path.

## Project structure

```
pathfinding-visualizer/
├── include/
│   └── MinHeap.hpp     # from-scratch templated binary min-heap  <- the DSA core
├── src/
│   └── main.cpp        # grid, BFS/DFS/Dijkstra/A*, terminal rendering
├── Makefile
└── README.md
```

## Possible extensions
- **Weighted terrain** — Dijkstra/A\* already generalise beyond unit costs.
- **Maze generation** — recursive division or randomized Prim's.
- **Diagonal movement** — extend the neighbour set and use an octile heuristic.
