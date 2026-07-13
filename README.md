# Pathfinding Visualizer

An interactive visualizer for four classic graph-search algorithms — **BFS, DFS,
Dijkstra, and A\*** — with the algorithms **and the min-heap priority queue that
powers them implemented from scratch** in vanilla JavaScript (no libraries, no
frameworks).

Draw walls, drop a start and finish, pick an algorithm, and watch it search the
grid cell by cell, then trace the shortest path.

> **Why this project:** it's a hands-on study of how uninformed search (BFS/DFS)
> compares to weighted (Dijkstra) and informed (A\*) search — including *why* A\*
> expands far fewer nodes. Run each algorithm on the same maze and compare the
> **"nodes visited"** counter to see it directly.

## Features
- **Four algorithms:** BFS, DFS, Dijkstra, A\* (Manhattan heuristic)
- **From-scratch data structures:** a binary **min-heap** priority queue drives
  Dijkstra and A\*; a queue drives BFS; a stack drives DFS
- **Interactive grid:** click-drag to draw walls; drag the start/finish squares
- **Live metrics:** nodes visited, final path length, and search status
- **Animated search + path**, with adjustable speed and a random-wall generator

## The DSA, and why each structure

| Algorithm | Data structure | Finds shortest path? | Idea |
|-----------|----------------|----------------------|------|
| **BFS**   | Queue (FIFO)   | ✅ (unweighted)      | Explore in rings of equal distance from the start |
| **DFS**   | Stack (LIFO)   | ❌                   | Dive deep down one branch before backtracking |
| **Dijkstra** | Min-heap    | ✅ (weighted)        | Always expand the closest unvisited node |
| **A\***   | Min-heap on `f = g + h` | ✅ (weighted) | Dijkstra guided toward the goal by a heuristic |

The **min-heap** is the heart of the project: a binary heap giving `O(log n)`
insert and extract-min, so Dijkstra and A\* always pull the next-best node
efficiently. It uses **lazy deletion** (skip already-finalized nodes when popped)
instead of a decrease-key operation, which keeps the structure simple while
staying correct.

## Complexity

Let `V` = cells, `E` = edges (≤ 4·V on a 4-connected grid).

| Algorithm | Time | Space |
|-----------|------|-------|
| BFS / DFS | `O(V + E)` | `O(V)` |
| Dijkstra  | `O(E log V)` | `O(V)` |
| A\*       | `O(E log V)` worst case; explores far fewer nodes with a good heuristic | `O(V)` |

A\* is optimal **and** faster than Dijkstra in practice because the Manhattan
heuristic is *admissible* (never overestimates), so it never expands a node it
can prove can't be on the best path.

## Run it

It's fully static — no build step, no dependencies.

```bash
# option 1: just open the file
open index.html

# option 2: serve it (needed if your browser blocks file:// for any asset)
python3 -m http.server 8000
# then visit http://localhost:8000
```

## Project structure

```
pathfinding-visualizer/
├── index.html          # layout, controls, legend
├── css/style.css       # grid + animations
└── js/
    ├── algorithms.js   # MinHeap + BFS/DFS/Dijkstra/A*  ← the DSA core
    └── app.js          # grid model, rendering, interaction, animation
```

## Try this
1. Click **Add Walls** to scatter obstacles.
2. Run **A\***, note the *nodes visited*.
3. Click **Reset Board**… actually — run **Dijkstra** on the *same* maze first
   and compare. Dijkstra explores in every direction; A\* beelines toward the
   goal. Same shortest path, very different amount of work. That contrast is the
   whole point.

## Notes / possible extensions
- Add **weighted terrain** (Dijkstra/A\* already handle non-uniform costs).
- Add **maze-generation** algorithms (recursive division, randomized Prim's).
- Allow **diagonal movement** (swap the neighbour function + use Euclidean/octile heuristic).
