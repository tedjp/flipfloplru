#include "flipfloplru.h"

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
    FlipFlopLRU<Movable> lru;

    lru.Put(Movable(1));

    return 0;
}
