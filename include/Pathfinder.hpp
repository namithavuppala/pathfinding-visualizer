#pragma once
#include <string>
#include <vector>

/*
 * Pathfinder — the reusable search engine shared by both front-ends:
 *   • src/main.cpp   → command-line / terminal visualizer
 *   • src/wasm.cpp   → WebAssembly binding for the browser visualizer
 *
 * The grid dimensions are dynamic (passed in the Grid), so the same code runs
 * a 20x40 terminal maze or a 25x51 browser maze without change.
 */

struct Cell { int r, c; };

struct Grid {
    int rows = 0;
    int cols = 0;
    std::vector<std::string> maze;   // rows strings of cols chars: '.' empty, '#' wall
    Cell start{0, 0};
    Cell finish{0, 0};
};

struct Result {
    std::vector<Cell> order;         // cells in expansion order (drives animation)
    std::vector<Cell> path;          // shortest path start->finish (empty if none)
    bool found = false;
};

// Individual algorithms.
Result bfs(const Grid& g);
Result dfs(const Grid& g);
Result dijkstra(const Grid& g);
Result astar(const Grid& g);

// Dispatch by name ("bfs" | "dfs" | "dijkstra" | "astar"); defaults to A*.
Result solveGrid(const Grid& g, const std::string& algo);
