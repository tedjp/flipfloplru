#include "flipfloplru.h"

struct Movable {
    Movable() = default;
    Movable(const Movable&) = delete;
    Movable(Movable&&) = default;
    Movable& operator=(const Movable&) = delete;
    Movable& operator=(Movable&&) = default;
};

int main(void) {
    FlipFlopLRU<Movable> lru;
    return 0;
}
