#include <unordered_map>

template <typename Key, typename Value>
class FlipFlopLRU {
public:
    Value* Get(const Key& key);
    void Put(const K& key, V&& element);

private:
    void Swap();

    std::unordered_map<Key, Value> left_, right_;
    std::unordered_map<Key, Value> *active_ = &left_, *passive_ = &right_;

    size_t max_size_ = 0;
};


template <typename Key, typename Value>
Value* FlipFlopLRU<key, Value>::Get(const Key& key) {
    auto found = active_->find(key);
    if (found != active_->end()) {
        return found->second;
    }

    found = passive_->find(key);
    if (found == passive_->end()) {
        return nullptr_;
    }
    else {
        found = active_->insert(key, move(found->second));
        return &found->second;
    }


    return nullptr;
}

template <typename Key, typename Value>
void FlipFlopLRU<Key, Value>::Put(const Key& key, Value&& value) {
    if (max_size_ == active->size())
    {
        Swap();
        passive_->clear();
    }

    active_->emplace(key, move(value));
}

template <typename Key, typename Value>
void FlipFlopLRU<key, Value>::Swap() {
    std::swap(active_, passive_);
    passive.clear();
}
