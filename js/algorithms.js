/*
 * algorithms.js — the DSA core of this project.
 *
 * Everything here is implemented from scratch (no libraries):
 *   • MinHeap        — a binary-heap priority queue (O(log n) push/pop)
 *   • BFS / DFS      — uninformed search over the grid graph
 *   • Dijkstra       — weighted shortest path using the MinHeap
 *   • A*             — Dijkstra + Manhattan heuristic (informed search)
 *
 * Each search returns { visitedOrder, path }:
 *   visitedOrder — nodes in the order they were expanded (drives the animation)
 *   path         — the final shortest path, start → finish (empty if unreachable)
 */

/* ----------------------------- MinHeap ----------------------------------- */
/* A binary min-heap. `scoreFn(node)` returns the key to order by, so the same
 * structure powers both Dijkstra (key = distance) and A* (key = distance + h).
 * We use lazy deletion (skip already-visited nodes on pop) instead of a
 * decrease-key operation, which keeps the heap simple and still correct. */
class MinHeap {
  constructor(scoreFn) {
    this.items = [];
    this.scoreFn = scoreFn;
  }
  size() { return this.items.length; }

  push(item) {
    this.items.push(item);
    this._bubbleUp(this.items.length - 1);
  }

  pop() {
    const top = this.items[0];
    const last = this.items.pop();
    if (this.items.length) { this.items[0] = last; this._bubbleDown(0); }
    return top;
  }

  _bubbleUp(i) {
    while (i > 0) {
      const parent = (i - 1) >> 1;
      if (this.scoreFn(this.items[i]) >= this.scoreFn(this.items[parent])) break;
      [this.items[i], this.items[parent]] = [this.items[parent], this.items[i]];
      i = parent;
    }
  }

  _bubbleDown(i) {
    const n = this.items.length;
    while (true) {
      let smallest = i;
      const l = 2 * i + 1, r = 2 * i + 2;
      if (l < n && this.scoreFn(this.items[l]) < this.scoreFn(this.items[smallest])) smallest = l;
      if (r < n && this.scoreFn(this.items[r]) < this.scoreFn(this.items[smallest])) smallest = r;
      if (smallest === i) break;
      [this.items[i], this.items[smallest]] = [this.items[smallest], this.items[i]];
      i = smallest;
    }
  }
}

/* --------------------------- graph helpers ------------------------------- */
/* 4-directional neighbours (up, right, down, left). */
function getNeighbors(node, grid) {
  const { row, col } = node;
  const out = [];
  if (row > 0) out.push(grid[row - 1][col]);
  if (col < grid[0].length - 1) out.push(grid[row][col + 1]);
  if (row < grid.length - 1) out.push(grid[row + 1][col]);
  if (col > 0) out.push(grid[row][col - 1]);
  return out;
}

function manhattan(a, b) {
  return Math.abs(a.row - b.row) + Math.abs(a.col - b.col);
}

/* Walk previousNode pointers from finish back to start. */
function reconstructPath(finish) {
  const path = [];
  let cur = finish;
  while (cur) { path.unshift(cur); cur = cur.previousNode; }
  return path;
}

/* -------------------------------- BFS ------------------------------------ */
/* Queue-based. On an unweighted graph BFS finds the shortest path (fewest
 * cells). Time O(V + E), space O(V). */
function bfs(grid, start, finish) {
  const visitedOrder = [];
  const queue = [start];
  start.visited = true;
  while (queue.length) {
    const node = queue.shift();          // dequeue (front)
    visitedOrder.push(node);
    if (node === finish) return { visitedOrder, path: reconstructPath(finish) };
    for (const nb of getNeighbors(node, grid)) {
      if (nb.visited || nb.isWall) continue;
      nb.visited = true;
      nb.previousNode = node;
      queue.push(nb);                    // enqueue (back)
    }
  }
  return { visitedOrder, path: [] };
}

/* -------------------------------- DFS ------------------------------------ */
/* Stack-based. Explores as deep as possible first — does NOT guarantee a
 * shortest path; included to contrast with BFS. Time O(V + E). */
function dfs(grid, start, finish) {
  const visitedOrder = [];
  const stack = [start];
  while (stack.length) {
    const node = stack.pop();            // LIFO
    if (node.visited || node.isWall) continue;
    node.visited = true;
    visitedOrder.push(node);
    if (node === finish) return { visitedOrder, path: reconstructPath(finish) };
    for (const nb of getNeighbors(node, grid)) {
      if (nb.visited || nb.isWall) continue;
      nb.previousNode = node;
      stack.push(nb);
    }
  }
  return { visitedOrder, path: [] };
}

/* ------------------------------ Dijkstra --------------------------------- */
/* Uniform edge weight of 1 here, so it behaves like a "careful" BFS but via a
 * priority queue — the structure that generalises to weighted graphs.
 * Time O(E log V) with a binary heap. */
function dijkstra(grid, start, finish) {
  const visitedOrder = [];
  start.distance = 0;
  const heap = new MinHeap(n => n.distance);
  heap.push(start);
  while (heap.size()) {
    const node = heap.pop();
    if (node.visited || node.isWall) continue;   // lazy deletion
    node.visited = true;
    visitedOrder.push(node);
    if (node === finish) return { visitedOrder, path: reconstructPath(finish) };
    for (const nb of getNeighbors(node, grid)) {
      if (nb.visited || nb.isWall) continue;
      const alt = node.distance + 1;
      if (alt < nb.distance) {
        nb.distance = alt;
        nb.previousNode = node;
        heap.push(nb);
      }
    }
  }
  return { visitedOrder, path: [] };
}

/* --------------------------------- A* ------------------------------------ */
/* Dijkstra guided by an admissible heuristic (Manhattan distance), so it
 * expands far fewer nodes toward the goal. Priority key f = g + h.
 * Time O(E log V), but explores much less of the grid in practice. */
function astar(grid, start, finish) {
  const visitedOrder = [];
  start.distance = 0;                              // g
  start.heuristic = manhattan(start, finish);      // h
  const heap = new MinHeap(n => n.distance + n.heuristic);  // f = g + h
  heap.push(start);
  while (heap.size()) {
    const node = heap.pop();
    if (node.visited || node.isWall) continue;
    node.visited = true;
    visitedOrder.push(node);
    if (node === finish) return { visitedOrder, path: reconstructPath(finish) };
    for (const nb of getNeighbors(node, grid)) {
      if (nb.visited || nb.isWall) continue;
      const alt = node.distance + 1;
      if (alt < nb.distance) {
        nb.distance = alt;
        nb.heuristic = manhattan(nb, finish);
        nb.previousNode = node;
        heap.push(nb);
      }
    }
  }
  return { visitedOrder, path: [] };
}

const ALGORITHMS = { bfs, dfs, dijkstra, astar };
