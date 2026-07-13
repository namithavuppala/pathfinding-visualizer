/*
 * Pathfinding Visualizer (C++) — BFS, DFS, Dijkstra, and A* on a grid.
 *
 * The min-heap that powers Dijkstra/A* is implemented from scratch in
 * include/MinHeap.hpp. BFS uses a queue, DFS uses a stack.
 *
 * Usage:
 *   ./pathfinder [bfs|dfs|dijkstra|astar|all] [--animate]
 *     (default: all — runs every algorithm and prints a comparison table)
 */
#include "MinHeap.hpp"

#include <array>
#include <chrono>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <thread>
#include <vector>

/* ------------------------------- model ----------------------------------- */
constexpr int ROWS = 20;
constexpr int COLS = 40;

struct Cell { int r, c; };

struct Grid {
    std::vector<std::string> maze;           // '.' empty, '#' wall
    Cell start{ROWS / 2, 4};
    Cell finish{ROWS / 2, COLS - 5};
};

struct Result {
    std::vector<Cell> order;                 // expansion order (for animation)
    std::vector<Cell> path;                  // final path start->finish
    bool found = false;
};

// Priority-queue element for Dijkstra / A*. `priority` is snapshotted so the
// heap needs no decrease-key (see MinHeap.hpp).
struct HeapItem { int priority; int r, c; };
struct ByPriority {
    bool operator()(const HeapItem& a, const HeapItem& b) const {
        return a.priority < b.priority;      // smaller priority = higher
    }
};

/* ------------------------------ helpers ---------------------------------- */
static const std::array<int, 4> DR = {-1, 0, 1, 0};
static const std::array<int, 4> DC = {0, 1, 0, -1};

bool inBounds(int r, int c) { return r >= 0 && r < ROWS && c >= 0 && c < COLS; }
int manhattan(Cell a, Cell b) { return std::abs(a.r - b.r) + std::abs(a.c - b.c); }

Grid makeGrid(double wallDensity = 0.20, unsigned seed = 42) {
    Grid g;
    g.maze.assign(ROWS, std::string(COLS, '.'));
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> chance(0.0, 1.0);
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            if (chance(rng) < wallDensity) g.maze[r][c] = '#';
    g.maze[g.start.r][g.start.c] = '.';
    g.maze[g.finish.r][g.finish.c] = '.';
    return g;
}

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

/* -------------------------------- BFS ------------------------------------ */
/* Queue (FIFO). Shortest path on an unweighted grid. O(V + E). */
Result bfs(const Grid& g) {
    Result res;
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    std::vector<std::vector<Cell>> prev(ROWS, std::vector<Cell>(COLS, {-1, -1}));
    std::queue<Cell> q;
    q.push(g.start);
    visited[g.start.r][g.start.c] = true;
    while (!q.empty()) {
        Cell cur = q.front(); q.pop();
        res.order.push_back(cur);
        if (cur.r == g.finish.r && cur.c == g.finish.c) { res.found = true; break; }
        for (int d = 0; d < 4; ++d) {
            int nr = cur.r + DR[d], nc = cur.c + DC[d];
            if (!inBounds(nr, nc) || visited[nr][nc] || g.maze[nr][nc] == '#') continue;
            visited[nr][nc] = true;
            prev[nr][nc] = cur;
            q.push({nr, nc});
        }
    }
    if (res.found) res.path = reconstruct(prev, g.finish);
    return res;
}

/* -------------------------------- DFS ------------------------------------ */
/* Stack (LIFO). Not shortest — included for contrast. O(V + E). */
Result dfs(const Grid& g) {
    Result res;
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    std::vector<std::vector<Cell>> prev(ROWS, std::vector<Cell>(COLS, {-1, -1}));
    std::vector<Cell> stack{g.start};
    while (!stack.empty()) {
        Cell cur = stack.back(); stack.pop_back();
        if (visited[cur.r][cur.c]) continue;
        visited[cur.r][cur.c] = true;
        res.order.push_back(cur);
        if (cur.r == g.finish.r && cur.c == g.finish.c) { res.found = true; break; }
        for (int d = 0; d < 4; ++d) {
            int nr = cur.r + DR[d], nc = cur.c + DC[d];
            if (!inBounds(nr, nc) || visited[nr][nc] || g.maze[nr][nc] == '#') continue;
            prev[nr][nc] = cur;
            stack.push_back({nr, nc});
        }
    }
    if (res.found) res.path = reconstruct(prev, g.finish);
    return res;
}

/* ----------------------------- Dijkstra ---------------------------------- */
/* Min-heap on distance. Uniform weight 1 here. O(E log V). */
Result dijkstra(const Grid& g) {
    Result res;
    std::vector<std::vector<int>> dist(ROWS, std::vector<int>(COLS, INT_MAX));
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    std::vector<std::vector<Cell>> prev(ROWS, std::vector<Cell>(COLS, {-1, -1}));
    MinHeap<HeapItem, ByPriority> heap;
    dist[g.start.r][g.start.c] = 0;
    heap.push({0, g.start.r, g.start.c});
    while (!heap.empty()) {
        HeapItem it = heap.pop();
        Cell cur{it.r, it.c};
        if (visited[cur.r][cur.c]) continue;               // lazy deletion
        visited[cur.r][cur.c] = true;
        res.order.push_back(cur);
        if (cur.r == g.finish.r && cur.c == g.finish.c) { res.found = true; break; }
        for (int d = 0; d < 4; ++d) {
            int nr = cur.r + DR[d], nc = cur.c + DC[d];
            if (!inBounds(nr, nc) || visited[nr][nc] || g.maze[nr][nc] == '#') continue;
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
/* Min-heap on f = g + h, Manhattan heuristic. Explores far fewer nodes. */
Result astar(const Grid& g) {
    Result res;
    std::vector<std::vector<int>> dist(ROWS, std::vector<int>(COLS, INT_MAX));
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLS, false));
    std::vector<std::vector<Cell>> prev(ROWS, std::vector<Cell>(COLS, {-1, -1}));
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
            if (!inBounds(nr, nc) || visited[nr][nc] || g.maze[nr][nc] == '#') continue;
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

/* ------------------------------ rendering -------------------------------- */
namespace color {
    const char* RESET = "\033[0m";
    const char* GREEN = "\033[42m";   // start
    const char* RED   = "\033[41m";   // finish
    const char* GRAY  = "\033[47m";   // wall
    const char* BLUE  = "\033[44m";   // visited
    const char* YEL   = "\033[43m";   // path
}

void render(const Grid& g,
            const std::vector<std::vector<char>>& overlay) { // 'V' visited, 'P' path
    std::string out;
    out += "\033[H";                                          // cursor home
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            const char* col;
            if (r == g.start.r && c == g.start.c)        col = color::GREEN;
            else if (r == g.finish.r && c == g.finish.c) col = color::RED;
            else if (g.maze[r][c] == '#')                col = color::GRAY;
            else if (overlay[r][c] == 'P')               col = color::YEL;
            else if (overlay[r][c] == 'V')               col = color::BLUE;
            else                                         col = color::RESET;
            out += col; out += "  "; out += color::RESET;
        }
        out += '\n';
    }
    std::cout << out << std::flush;
}

void animate(const Grid& g, const Result& res) {
    std::vector<std::vector<char>> overlay(ROWS, std::vector<char>(COLS, ' '));
    std::cout << "\033[2J";                                   // clear screen once
    for (std::size_t i = 0; i < res.order.size(); ++i) {
        Cell cell = res.order[i];
        overlay[cell.r][cell.c] = 'V';
        if (i % 3 == 0 || i + 1 == res.order.size()) {
            render(g, overlay);
            std::this_thread::sleep_for(std::chrono::milliseconds(12));
        }
    }
    for (Cell cell : res.path) overlay[cell.r][cell.c] = 'P';
    render(g, overlay);
}

void renderFinal(const Grid& g, const Result& res) {
    std::vector<std::vector<char>> overlay(ROWS, std::vector<char>(COLS, ' '));
    for (Cell cell : res.order) overlay[cell.r][cell.c] = 'V';
    for (Cell cell : res.path)  overlay[cell.r][cell.c] = 'P';
    std::cout << "\033[2J";
    render(g, overlay);
}

/* -------------------------------- driver --------------------------------- */
Result run(const std::string& algo, const Grid& g) {
    if (algo == "bfs")      return bfs(g);
    if (algo == "dfs")      return dfs(g);
    if (algo == "dijkstra") return dijkstra(g);
    return astar(g);                                          // default
}

void printStats(const std::string& name, const Result& r) {
    std::cout << "  " << name;
    for (std::size_t i = name.size(); i < 12; ++i) std::cout << ' ';
    std::cout << "visited: " << r.order.size();
    std::cout << "\tpath: " << (r.found ? std::to_string(r.path.size() - 1) : std::string("no path"))
              << "\n";
}

int main(int argc, char** argv) {
    std::string algo = argc > 1 ? argv[1] : "all";
    bool doAnimate = false;
    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == "--animate") doAnimate = true;

    Grid g = makeGrid();

    if (algo == "all") {
        std::cout << "Pathfinding on a " << ROWS << "x" << COLS
                  << " grid (start -> finish), same maze for all four:\n\n";
        printStats("BFS",      bfs(g));
        printStats("DFS",      dfs(g));
        printStats("Dijkstra", dijkstra(g));
        printStats("A*",       astar(g));
        std::cout << "\nNote how A* visits the fewest nodes: the Manhattan "
                     "heuristic steers it toward the goal.\n"
                     "Run a single algorithm to watch it: ./pathfinder astar --animate\n";
        return 0;
    }

    Result res = run(algo, g);
    if (doAnimate) animate(g, res);
    else           renderFinal(g, res);
    std::cout << "\nAlgorithm: " << algo
              << "   nodes visited: " << res.order.size()
              << "   path length: "
              << (res.found ? std::to_string(res.path.size() - 1) : std::string("no path"))
              << "\n";
    return 0;
}
