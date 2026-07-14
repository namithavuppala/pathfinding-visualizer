/*
 * main.cpp — command-line / terminal front-end for the pathfinding engine.
 *
 * The search algorithms + the from-scratch min-heap live in Pathfinder.cpp /
 * MinHeap.hpp and are shared with the WebAssembly build (src/wasm.cpp). This
 * file only adds the random maze, the ANSI terminal rendering, and the CLI.
 *
 * Usage:
 *   ./pathfinder [bfs|dfs|dijkstra|astar|all] [--animate]
 *     (default: all — runs every algorithm and prints a comparison table)
 */
#include "Pathfinder.hpp"

#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

constexpr int ROWS = 20;
constexpr int COLS = 40;

Grid makeGrid(double wallDensity = 0.20, unsigned seed = 42) {
    Grid g;
    g.rows = ROWS;
    g.cols = COLS;
    g.maze.assign(ROWS, std::string(COLS, '.'));
    g.start = {ROWS / 2, 4};
    g.finish = {ROWS / 2, COLS - 5};
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> chance(0.0, 1.0);
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            if (chance(rng) < wallDensity) g.maze[r][c] = '#';
    g.maze[g.start.r][g.start.c] = '.';
    g.maze[g.finish.r][g.finish.c] = '.';
    return g;
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

void render(const Grid& g, const std::vector<std::vector<char>>& overlay) {
    std::string out = "\033[H";                              // cursor home
    for (int r = 0; r < g.rows; ++r) {
        for (int c = 0; c < g.cols; ++c) {
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
    std::vector<std::vector<char>> overlay(g.rows, std::vector<char>(g.cols, ' '));
    std::cout << "\033[2J";
    for (std::size_t i = 0; i < res.order.size(); ++i) {
        overlay[res.order[i].r][res.order[i].c] = 'V';
        if (i % 3 == 0 || i + 1 == res.order.size()) {
            render(g, overlay);
            std::this_thread::sleep_for(std::chrono::milliseconds(12));
        }
    }
    for (Cell cell : res.path) overlay[cell.r][cell.c] = 'P';
    render(g, overlay);
}

void renderFinal(const Grid& g, const Result& res) {
    std::vector<std::vector<char>> overlay(g.rows, std::vector<char>(g.cols, ' '));
    for (Cell cell : res.order) overlay[cell.r][cell.c] = 'V';
    for (Cell cell : res.path)  overlay[cell.r][cell.c] = 'P';
    std::cout << "\033[2J";
    render(g, overlay);
}

void printStats(const std::string& name, const Result& r) {
    std::cout << "  " << name;
    for (std::size_t i = name.size(); i < 12; ++i) std::cout << ' ';
    std::cout << "visited: " << r.order.size();
    std::cout << "\tpath: "
              << (r.found ? std::to_string(r.path.size() - 1) : std::string("no path")) << "\n";
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

    Result res = solveGrid(g, algo);
    if (doAnimate) animate(g, res);
    else           renderFinal(g, res);
    std::cout << "\nAlgorithm: " << algo
              << "   nodes visited: " << res.order.size()
              << "   path length: "
              << (res.found ? std::to_string(res.path.size() - 1) : std::string("no path"))
              << "\n";
    return 0;
}
