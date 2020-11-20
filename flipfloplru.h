#include <unordered_map>

template <typename Key, typename Value>
class FlipFlopLRU final {
public:
    explicit FlipFlopLRU(size_t max_size);

    FlipFlopLRU(const FlipFlopLRU&);
    FlipFlopLRU(FlipFlopLRU&&);
    FlipFlopLRU<Key, Value>& operator=(const FlipFlopLRU&);
    FlipFlopLRU<Key, Value>& operator=(FlipFlopLRU&&);
    ~FlipFlopLRU() = default;

    void clear() noexcept;
    bool contains(const Key& key) const noexcept;
    Value* get(const Key& key);
    Value& put(const Key& key, Value&& element);
    //Value& emplace(...);
    //Value& operator[](const Key& key);
    //Value& operator[](Key&& key);

    // Returns the size of the active & passive containers
    std::pair<size_t, size_t> size() const noexcept;

private:
    void cloneActivePassivePointers(const FlipFlopLRU& other) noexcept;
    void flipFlop() noexcept;

    std::unordered_map<Key, Value> left_, right_;
    std::unordered_map<Key, Value> *active_ = &left_, *passive_ = &right_;

    size_t max_size_ = 0;
};

template <typename Key, typename Value>
FlipFlopLRU<Key, Value>::FlipFlopLRU(size_t max_size):
    max_size_(max_size)
{
    left_.reserve(max_size);
    right_.reserve(max_size);
}

template <typename Key, typename Value>
FlipFlopLRU<Key, Value>::FlipFlopLRU(const FlipFlopLRU& other):
    left_(other.left_),
    right_(other.right_),
    active_(nullptr),
    passive_(nullptr)
{
    cloneActivePassivePointers(other);
}

template <typename Key, typename Value>
FlipFlopLRU<Key, Value>::FlipFlopLRU(FlipFlopLRU&& other):
    left_(std::move(other.left_)),
    right_(std::move(other.right_)),
    active_(nullptr),
    passive_(nullptr)
{
    cloneActivePassivePointers(other);
}

template <typename Key, typename Value>
FlipFlopLRU<Key, Value>& FlipFlopLRU<Key, Value>::operator=(const FlipFlopLRU& other) {
    if (&other == this)
        return *this;

    left_ = other.left_;
    right_ = other.right_;
    cloneActivePassivePointers(other);
    return *this;
}

template <typename Key, typename Value>
FlipFlopLRU<Key, Value>& FlipFlopLRU<Key, Value>::operator=(FlipFlopLRU&& other) {
    if (&other == this)
        return *this;

    left_ = std::move(other.left_);
    right_ = std::move(other.right_);
    cloneActivePassivePointers(other);
    return *this;
}

template <typename Key, typename Value>
void FlipFlopLRU<Key, Value>::clear() noexcept {
    passive_->clear();
    active_->clear();
}

template <typename Key, typename Value>
bool FlipFlopLRU<Key, Value>::contains(const Key& key) const noexcept {
    return active_->find(key) != active_->end()
        || passive_->find(key) != passive_->end();
}

template <typename Key, typename Value>
Value* FlipFlopLRU<Key, Value>::get(const Key& key) {
    auto found = active_->find(key);
    if (found != active_->end())
        return found->second;

    found = passive_->find(key);
    if (found == passive_->end())
        return nullptr;

    if (active_->size() == max_size_) {
        auto elem = move(found->second);
        flipFlop();
        return &active_->emplace(key, move(elem)).first;
    }

    found = active_->insert(key, move(found->second));
    return &found->second;
}

template <typename Key, typename Value>
Value& FlipFlopLRU<Key, Value>::put(const Key& key, Value&& value) {
    if (active_->size() >= max_size_)
        flipFlop();

    auto placed = active_->emplace(key, std::move(value));
    return placed.first->second;
}

template <typename Key, typename Value>
std::pair<size_t, size_t> FlipFlopLRU<Key, Value>::size() const noexcept {
    return {active_->size(), passive_->size()};
}

template <typename Key, typename Value>
void FlipFlopLRU<Key, Value>::cloneActivePassivePointers(const FlipFlopLRU& other) noexcept {
    if (other.active_ == &other.left_) {
        active_ = &left_;
        passive_ = &right_;
    } else {
        active_ = &right_;
        passive_ = &left_;
    }
}

template <typename Key, typename Value>
void FlipFlopLRU<Key, Value>::flipFlop() noexcept {
    passive_->clear();
    std::swap(active_, passive_);
}
