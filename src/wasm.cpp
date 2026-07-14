/*
 * wasm.cpp — Emscripten binding that exposes the C++ search engine to the
 * browser. Compiled to WebAssembly; the JavaScript front-end calls solve()
 * and the actual BFS/DFS/Dijkstra/A* + min-heap run as compiled C++.
 *
 * Build (see build_wasm.sh):
 *   emcc -std=c++17 -O2 -Iinclude src/Pathfinder.cpp src/wasm.cpp \
 *        -lembind -sMODULARIZE=1 -sEXPORT_NAME=PathfinderModule -o docs/pathfinder.js
 */
#include "Pathfinder.hpp"

#include <emscripten/bind.h>
#include <sstream>
#include <string>

/*
 * walls: a string of length rows*cols, '1' = wall, '0' = open (row-major).
 * Returns a JSON string: {"found":bool,"order":[[r,c],...],"path":[[r,c],...]}
 * so the JS side just needs JSON.parse — no manual memory handling.
 */
std::string solve(const std::string& walls, int rows, int cols,
                  int sr, int sc, int fr, int fc, const std::string& algo) {
    Grid g;
    g.rows = rows;
    g.cols = cols;
    g.maze.assign(rows, std::string(cols, '.'));
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            if (walls[static_cast<std::size_t>(r) * cols + c] == '1')
                g.maze[r][c] = '#';
    g.start = {sr, sc};
    g.finish = {fr, fc};

    Result res = solveGrid(g, algo);

    std::ostringstream os;
    os << "{\"found\":" << (res.found ? "true" : "false") << ",\"order\":[";
    for (std::size_t i = 0; i < res.order.size(); ++i) {
        if (i) os << ",";
        os << "[" << res.order[i].r << "," << res.order[i].c << "]";
    }
    os << "],\"path\":[";
    for (std::size_t i = 0; i < res.path.size(); ++i) {
        if (i) os << ",";
        os << "[" << res.path[i].r << "," << res.path[i].c << "]";
    }
    os << "]}";
    return os.str();
}

EMSCRIPTEN_BINDINGS(pathfinder) {
    emscripten::function("solve", &solve);
}
