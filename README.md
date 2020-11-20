# Flip-flop LRU Cache

Efficient C++ LRU cache. Gets usually don't cause any memory writes or internal
cache restructuring (ie. no linked-list). Optimal workloads with 100% hit rate
cause zero memory writes.

This is a header-only component; only [flipfloplru.h](flipfloplru.h) is
required.

# License
[MIT License](LICENSE).
