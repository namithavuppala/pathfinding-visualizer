# Pathfinding Visualizer (C++ core, terminal + WebAssembly front-ends)

Four classic graph-search algorithms (BFS, DFS, Dijkstra, and A\*) with the
**binary min-heap priority queue implemented from scratch** in modern C++ (C++17).
One engine, two front-ends:

- **Terminal** (`./pathfinder`): animated ANSI grid, right in your shell.
- **Browser** (`docs/`): the same C++ engine compiled to WebAssembly. The web
  page draws the grid and animates the result, but every search is computed by
  the compiled C++ (`solve()` in `src/wasm.cpp`), not by JavaScript.

## The DSA, and why each structure

| Algorithm | Data structure | Shortest path | Idea |
|-----------|----------------|---------------|------|
| **BFS**   | Queue (FIFO)   | yes (unweighted) | Explore in rings of equal distance |
| **DFS**   | Stack (LIFO)   | no               | Dive deep down one branch first |
| **Dijkstra** | Custom min-heap | yes (weighted) | Always expand the closest node |
| **A\***   | Min-heap on `f = g + h` | yes (weighted) | Dijkstra guided toward the goal |

The templated **`MinHeap`** in [`include/MinHeap.hpp`](include/MinHeap.hpp) is the
heart of the project: a binary heap giving `O(log n)` push/pop. Dijkstra and A\*
use **lazy deletion**, where a node may be pushed again when a shorter route is
found and stale copies are skipped on pop, so there is no decrease-key operation.

## Sample results (25x51 grid, straight-line start to finish, no walls)

Measured by calling the C++ engine directly from the browser (`engine.solve`):

| Algorithm | Nodes visited | Path length |
|-----------|---------------|-------------|
| A\*       | 35   | 34 |
| Dijkstra  | 895  | 34 |
| BFS       | 907  | 34 |
| DFS       | 655  | 646 |

BFS, Dijkstra, and A\* all reach the same optimal length (34), but A\* gets there
after exploring roughly 25 times fewer cells, because the admissible Manhattan
heuristic steers it toward the goal. DFS finds a path, but a hugely suboptimal one.

## Complexity

Let `V` = cells, `E` = edges (at most 4V on a 4-connected grid).

| Algorithm | Time | Space |
|-----------|------|-------|
| BFS / DFS | `O(V + E)` | `O(V)` |
| Dijkstra  | `O(E log V)` | `O(V)` |
| A\*       | `O(E log V)` worst case; explores far fewer nodes with a good heuristic | `O(V)` |

## Run the terminal version

Requires a C++17 compiler (`g++` or `clang++`). No dependencies.

```bash
make                           # builds ./pathfinder
./pathfinder all               # run all four, print the comparison table
./pathfinder astar --animate   # watch A* search the grid live
```

## Run the browser version (C++ to WebAssembly)

Requires [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)
(`emcc`) to build the WebAssembly module:

```bash
# one-time: install + activate Emscripten, then:
source /path/to/emsdk/emsdk_env.sh

./build_wasm.sh                # produces docs/pathfinder.js + docs/pathfinder.wasm

cd docs && python3 -m http.server 8000
# open http://localhost:8000
```

The prebuilt `docs/pathfinder.js` and `docs/pathfinder.wasm` are committed, so the
site also works when served straight from GitHub Pages, no build needed to view.

## How the browser version works

```
 index.html / app.js  --(walls, start, finish, algorithm)-->  engine.solve()
   (draws grid,                                                = C++ compiled
    animates result)  <--(JSON: visited order + path)--------   to WebAssembly
```

`app.js` implements no pathfinding of its own. It packs the grid into a string,
calls into the WebAssembly module, and animates the `{order, path}` the C++ returns.

## Project structure

```
pathfinding-visualizer/
├── include/
│   ├── MinHeap.hpp      # from-scratch templated binary min-heap (the DSA core)
│   └── Pathfinder.hpp   # Grid / Result types + algorithm declarations
├── src/
│   ├── Pathfinder.cpp   # BFS / DFS / Dijkstra / A*  (shared engine)
│   ├── main.cpp         # terminal front-end (ANSI rendering)
│   └── wasm.cpp         # Emscripten binding: solve() to JSON
├── docs/                # browser front-end (GitHub Pages root)
│   ├── index.html, app.js, style.css
│   └── pathfinder.js, pathfinder.wasm   (built by build_wasm.sh)
├── build_wasm.sh
├── Makefile
└── README.md
```

## Possible extensions
- **Weighted terrain**: Dijkstra/A\* already generalise beyond unit costs.
- **Maze generation**: recursive division or randomized Prim's.
- **Diagonal movement**: extend the neighbour set and use an octile heuristic.
