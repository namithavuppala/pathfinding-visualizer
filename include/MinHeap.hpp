#pragma once
#include <vector>
#include <functional>
#include <cstddef>

/*
 * MinHeap<T, Compare> — a binary min-heap priority queue built from scratch.
 *
 * This is the core data structure of the project: it gives O(log n) push and
 * pop, which is what lets Dijkstra and A* always expand the next-best node
 * efficiently. `Compare(a, b)` returns true when `a` has higher priority
 * (should sit closer to the top) than `b`.
 *
 * Dijkstra / A* use "lazy deletion": a node may be pushed more than once as a
 * shorter path is found, and stale copies are simply skipped when popped. That
 * keeps the heap simple (no decrease-key) while staying correct.
 */
template <typename T, typename Compare = std::less<T>>
class MinHeap {
public:
    explicit MinHeap(Compare cmp = Compare()) : cmp_(cmp) {}

    bool empty() const { return data_.empty(); }
    std::size_t size() const { return data_.size(); }

    void push(const T& value) {
        data_.push_back(value);
        bubbleUp(data_.size() - 1);
    }

    // Precondition: heap is non-empty.
    T pop() {
        T top = data_.front();
        data_.front() = data_.back();
        data_.pop_back();
        if (!data_.empty()) bubbleDown(0);
        return top;
    }

private:
    std::vector<T> data_;
    Compare cmp_;

    void bubbleUp(std::size_t i) {
        while (i > 0) {
            std::size_t parent = (i - 1) / 2;
            if (!cmp_(data_[i], data_[parent])) break;   // parent already >= child
            std::swap(data_[i], data_[parent]);
            i = parent;
        }
    }

    void bubbleDown(std::size_t i) {
        const std::size_t n = data_.size();
        while (true) {
            std::size_t best = i;
            std::size_t l = 2 * i + 1, r = 2 * i + 2;
            if (l < n && cmp_(data_[l], data_[best])) best = l;
            if (r < n && cmp_(data_[r], data_[best])) best = r;
            if (best == i) break;
            std::swap(data_[i], data_[best]);
            i = best;
        }
    }
};
