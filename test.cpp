/**
 * Flip-flop LRU Cache.
 * Copyright 2020 Ted J. Percival.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
