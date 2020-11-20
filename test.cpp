#include "flipfloplru.h"

#include <cassert>
#include <limits>

struct Movable {
public:
    Movable() = default;
    explicit Movable(int v): v_(v) {}
    Movable(const Movable&) = delete;
    Movable(Movable&&) = default;
    Movable& operator=(const Movable&) = delete;
    Movable& operator=(Movable&&) = default;
private:
    int v_ = 0;
};

int main(void) {
    FlipFlopLRU<int, Movable> lru(1);

    // Test reserve()
    lru.reserve();

    // Test that reserve() clamps to actual maximum size.
    lru.reserve(std::numeric_limits<size_t>::max());

    // Test accessor (return value ignored)
    lru.maxSize();

    lru.put(1, Movable(1));
    Movable* m = lru.get(1);
    assert(m != nullptr);
    m = lru.get(2);
    assert(m == nullptr);
    lru.put(2, Movable(2));
    // Pull 1 from inactive side
    m = lru.get(1);
    assert(m != nullptr);
    // Replace 1
    lru.put(1, Movable(1));
    // Replace inactive-side element
    lru.put(2, Movable(2));

    // Move entire FlipFlopLRU
    FlipFlopLRU<int, Movable> lru2 = std::move(lru);
    FlipFlopLRU<int, Movable> lru3(std::move(lru2));

    return 0;
}
