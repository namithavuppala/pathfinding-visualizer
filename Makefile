CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Iinclude
TARGET    = pathfinder
SRC       = src/main.cpp src/Pathfinder.cpp
HEADERS   = include/Pathfinder.hpp include/MinHeap.hpp

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) all

# Build the WebAssembly module for the browser front-end (needs Emscripten).
wasm:
	./build_wasm.sh

clean:
	rm -f $(TARGET)

.PHONY: run wasm clean
