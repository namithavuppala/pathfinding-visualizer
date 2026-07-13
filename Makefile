CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Iinclude
TARGET    = pathfinder
SRC       = src/main.cpp

$(TARGET): $(SRC) include/MinHeap.hpp
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) all

clean:
	rm -f $(TARGET)

.PHONY: run clean
