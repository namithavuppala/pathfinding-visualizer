#!/usr/bin/env bash
# Build the WebAssembly module that powers the browser visualizer.
# Requires Emscripten (emcc) on PATH — see README "Build the WebAssembly module".
set -euo pipefail

mkdir -p docs

emcc -std=c++17 -O2 -Iinclude \
  src/Pathfinder.cpp src/wasm.cpp \
  -lembind \
  -sMODULARIZE=1 -sEXPORT_NAME=PathfinderModule -sENVIRONMENT=web \
  -o docs/pathfinder.js

echo "Built docs/pathfinder.js + docs/pathfinder.wasm"
