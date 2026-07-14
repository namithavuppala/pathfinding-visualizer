/*
 * Pathfinder.cpp — BFS, DFS, Dijkstra, and A* over a dynamic grid.
 * The min-heap that powers Dijkstra/A* is implemented from scratch in
 * include/MinHeap.hpp. BFS uses a queue, DFS uses a stack.
 */
#include "Pathfinder.hpp"
#include "MinHeap.hpp"

#include <algorithm>
#include <array>
#include <climits>
#include <cstdlib>
#include <queue>

namespace {

const std::array<int, 4> DR = {-1, 0, 1, 0};
const std::array<int, 4> DC = {0, 1, 0, -1};

bool inBounds(const Grid& g, int r, int c) {
    return r >= 0 && r < g.rows && c >= 0 && c < g.cols;
}
bool isWall(const Grid& g, int r, int c) { return g.maze[r][c] == '#'; }
int manhattan(Cell a, Cell b) { return std::abs(a.r - b.r) + std::abs(a.c - b.c); }

std::vector<Cell> reconstruct(const std::vector<std::vector<Cell>>& prev, Cell finish) {
    std::vector<Cell> path;
    Cell cur = finish;
    while (cur.r != -1) {
        path.push_back(cur);
        cur = prev[cur.r][cur.c];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

// Priority-queue element for Dijkstra / A*. `priority` is snapshotted so the
// heap needs no decrease-key (stale entries are skipped on pop).
struct HeapItem { int priority; int r, c; };
struct ByPriority {
    bool operator()(const HeapItem& a, const HeapItem& b) const {
        return a.priority < b.priority;
    }
};

}  // namespace

/* -------------------------------- BFS ------------------------------------ */
Result bfs(const Grid& g) {
    Result res;
    std::vector<std::vector<bool>> visited(g.rows, std::vector<bool>(g.cols, false));
    std::vector<std::vector<Cell>> prev(g.rows, std::vector<Cell>(g.cols, {-1, -1}));
    std::queue<Cell> q;
    q.push(g.start);
    visited[g.start.r][g.start.c] = true;
    while (!q.empty()) {
        Cell cur = q.front(); q.pop();
        res.order.push_back(cur);
        if (cur.r == g.finish.r && cur.c == g.finish.c) { res.found = true; break; }
        for (int d = 0; d < 4; ++d) {
            int nr = cur.r + DR[d], nc = cur.c + DC[d];
            if (!inBounds(g, nr, nc) || visited[nr][nc] || isWall(g, nr, nc)) continue;
            visited[nr][nc] = true;
            prev[nr][nc] = cur;
            q.push({nr, nc});
        }
    }
    if (res.found) res.path = reconstruct(prev, g.finish);
    return res;
}

/* -------------------------------- DFS ------------------------------------ */
Result dfs(const Grid& g) {
    Result res;
    std::vector<std::vector<bool>> visited(g.rows, std::vector<bool>(g.cols, false));
    std::vector<std::vector<Cell>> prev(g.rows, std::vector<Cell>(g.cols, {-1, -1}));
    std::vector<Cell> stack{g.start};
    while (!stack.empty()) {
        Cell cur = stack.back(); stack.pop_back();
        if (visited[cur.r][cur.c]) continue;
        visited[cur.r][cur.c] = true;
        res.order.push_back(cur);
        if (cur.r == g.finish.r && cur.c == g.finish.c) { res.found = true; break; }
        for (int d = 0; d < 4; ++d) {
            int nr = cur.r + DR[d], nc = cur.c + DC[d];
            if (!inBounds(g, nr, nc) || visited[nr][nc] || isWall(g, nr, nc)) continue;
            prev[nr][nc] = cur;
            stack.push_back({nr, nc});
        }
    }
    if (res.found) res.path = reconstruct(prev, g.finish);
    return res;
}

/* ----------------------------- Dijkstra ---------------------------------- */
Result dijkstra(const Grid& g) {
    Result res;
    std::vector<std::vector<int>> dist(g.rows, std::vector<int>(g.cols, INT_MAX));
    std::vector<std::vector<bool>> visited(g.rows, std::vector<bool>(g.cols, false));
    std::vector<std::vector<Cell>> prev(g.rows, std::vector<Cell>(g.cols, {-1, -1}));
    MinHeap<HeapItem, ByPriority> heap;
    dist[g.start.r][g.start.c] = 0;
    heap.push({0, g.start.r, g.start.c});
    while (!heap.empty()) {
        HeapItem it = heap.pop();
        Cell cur{it.r, it.c};
        if (visited[cur.r][cur.c]) continue;            // lazy deletion
        visited[cur.r][cur.c] = true;
        res.order.push_back(cur);
        if (cur.r == g.finish.r && cur.c == g.finish.c) { res.found = true; break; }
        for (int d = 0; d < 4; ++d) {
            int nr = cur.r + DR[d], nc = cur.c + DC[d];
            if (!inBounds(g, nr, nc) || visited[nr][nc] || isWall(g, nr, nc)) continue;
            int nd = dist[cur.r][cur.c] + 1;
            if (nd < dist[nr][nc]) {
                dist[nr][nc] = nd;
                prev[nr][nc] = cur;
                heap.push({nd, nr, nc});
            }
        }
    }
    if (res.found) res.path = reconstruct(prev, g.finish);
    return res;
}

/* -------------------------------- A* ------------------------------------- */
Result astar(const Grid& g) {
    Result res;
    std::vector<std::vector<int>> dist(g.rows, std::vector<int>(g.cols, INT_MAX));
    std::vector<std::vector<bool>> visited(g.rows, std::vector<bool>(g.cols, false));
    std::vector<std::vector<Cell>> prev(g.rows, std::vector<Cell>(g.cols, {-1, -1}));
    MinHeap<HeapItem, ByPriority> heap;
    dist[g.start.r][g.start.c] = 0;
    heap.push({manhattan(g.start, g.finish), g.start.r, g.start.c});
    while (!heap.empty()) {
        HeapItem it = heap.pop();
        Cell cur{it.r, it.c};
        if (visited[cur.r][cur.c]) continue;
        visited[cur.r][cur.c] = true;
        res.order.push_back(cur);
        if (cur.r == g.finish.r && cur.c == g.finish.c) { res.found = true; break; }
        for (int d = 0; d < 4; ++d) {
            int nr = cur.r + DR[d], nc = cur.c + DC[d];
            if (!inBounds(g, nr, nc) || visited[nr][nc] || isWall(g, nr, nc)) continue;
            int nd = dist[cur.r][cur.c] + 1;
            if (nd < dist[nr][nc]) {
                dist[nr][nc] = nd;
                prev[nr][nc] = cur;
                heap.push({nd + manhattan({nr, nc}, g.finish), nr, nc});
            }
        }
    }
    if (res.found) res.path = reconstruct(prev, g.finish);
    return res;
}

/* ------------------------------ dispatch --------------------------------- */
Result solveGrid(const Grid& g, const std::string& algo) {
    if (algo == "bfs")      return bfs(g);
    if (algo == "dfs")      return dfs(g);
    if (algo == "dijkstra") return dijkstra(g);
    return astar(g);
}
