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
        left_.reserve(max_size);
        right_.reserve(max_size);
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
        if (found != active_->end())
            return &found->second;

        found = passive_->find(key);
        if (found == passive_->end())
            return nullptr;

        if (active_->size() == max_size_) {
            // Future: Use extract() to move the entire node.
            auto elem = move(found->second);
            flipFlop();
            return &active_->emplace(key, move(elem)).first;
        }

        found = active_->insert(key, move(found->second));
        return &found->second;
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

    void flipFlop() noexcept {
        passive_->clear();
        std::swap(active_, passive_);
    }
};
