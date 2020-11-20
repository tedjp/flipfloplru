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
        // exceeds max_size_ (ie. never flip-flops), at the cost of a delay
        // to allocate during the first flip-flop. The delay can be avoided by
        // calling reserve() to explicitly reserve the full amount of space in
        // both underlying containers (active & passive).
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

    // Reserve space for a full cache.
    void reserve() {
        reserve(max_size_);
    }

    // Reserve space for a number of elements (potentially requiring resize
    // of the underlying containers later).
    // If size is greater than the max_size_, only reserves the max_size_.
    void reserve(size_t size) {
        size = std::min(size, max_size_);
        passive_->reserve(size);
        active_->reserve(size);
    }

    //Value& emplace(...);

    //Value& operator[](const Key& key);

    //Value& operator[](Key&& key);

    // Returns the size of the active & passive containers
    std::pair<size_t, size_t> size() const noexcept {
        return {active_->size(), passive_->size()};
    }

    // Return the maximum number of active elements. The actual number of
    // stored elements may be up to double due to unreleased inactive-side
    // elements.
    size_t maxSize() const noexcept {
        return max_size_;
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
