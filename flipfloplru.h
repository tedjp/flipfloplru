#include <unordered_map>

template <
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    typename KeyEqual = std::equal_to<Key>,
    typename Allocator = std::allocator<std::pair<const Key, Value>>
    >
class FlipFlopLRU final {
private:

    std::unordered_map<Key, Value> left_, right_;
    std::unordered_map<Key, Value> *active_ = &left_, *passive_ = &right_;

    size_t max_size_ = 0;

public:

    explicit FlipFlopLRU(size_t max_size):
        max_size_(max_size)
    {
        active_->reserve(max_size);
        // Passive side doesn't allocate until the first flipFlop().
        // This provides a smaller memory footprint when the dataset never
        // exceeds max_size_ (ie. never flip-flops).
    }

    FlipFlopLRU(const FlipFlopLRU& other):
        left_(other.left_),
        right_(other.right_),
        active_(nullptr),
        passive_(nullptr)
    {
        cloneActivePassivePointers(other);
    }

    FlipFlopLRU(FlipFlopLRU&& other):
        left_(std::move(other.left_)),
        right_(std::move(other.right_)),
        active_(nullptr),
        passive_(nullptr)
    {
        cloneActivePassivePointers(other);
    }

    FlipFlopLRU& operator=(const FlipFlopLRU& other) {
        // self-assignment safe
        left_ = other.left_;
        right_ = other.right_;
        cloneActivePassivePointers(other);
        return *this;
    }

    FlipFlopLRU& operator=(FlipFlopLRU&& other) {
        // self-assignment safe
        left_ = std::move(other.left_);
        right_ = std::move(other.right_);
        cloneActivePassivePointers(other);
        return *this;
    }

    ~FlipFlopLRU() = default;

    void clear() noexcept {
        passive_->clear();
        active_->clear();
    }

    bool contains(const Key& key) const noexcept {
        return active_->find(key) != active_->end()
            || passive_->find(key) != passive_->end();
    }

    Value* get(const Key& key) {
        auto found = active_->find(key);
        if (found != active_->end()) {
            // Found in active set
            return &found->second;
        }

        found = passive_->find(key);
        if (found == passive_->end()) {
            // Not in active or passive
            return nullptr;
        }

        if (active_->size() == max_size_) {
            // Found in passive, but active is already full.
            // Future: Use extract() to move the entire node.
            auto elem = std::move(found->second);
            flipFlop();
            auto placed = active_->emplace(key, std::move(elem));
            return &placed.first->second;
        }

        // Found in passive, move to active
        // Future: Use extract() to move the entire node.
        auto placed = active_->emplace(std::move(*found));
        return &placed.first->second;
    }

    Value& put(const Key& key, const Value& value) {
        if (active_->size() >= max_size_)
            flipFlop();

        auto placed = active_->insert_or_assign(key, value);
        return placed.first->second;
    }

    // if a duplicate exists in the passive set, it's left there harmlessly
    // (until the next flipFlop()).
    Value& put(const Key& key, Value&& value) {
        if (active_->size() >= max_size_)
            flipFlop();

        auto placed = active_->insert_or_assign(key, std::move(value));
        return placed.first->second;
    }

    //Value& emplace(...);

    //Value& operator[](const Key& key);

    //Value& operator[](Key&& key);

    // Returns the size of the active & passive containers
    std::pair<size_t, size_t> size() const noexcept {
        return {active_->size(), passive_->size()};
    }

private:

    void cloneActivePassivePointers(const FlipFlopLRU& other) noexcept {
        if (other.active_ == &other.left_) {
            active_ = &left_;
            passive_ = &right_;
        } else {
            active_ = &right_;
            passive_ = &left_;
        }
    }

    void flipFlop() {
        passive_->clear();
        std::swap(active_, passive_);

        // Delayed allocation of the 2nd underlying container until the first
        // flipFlop. This will be a no-op on subsequent flipFlops.
        active_->reserve(max_size_);
    }
};
