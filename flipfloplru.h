#include <vector>

template <typename T>
class FlipFlopLRU {
    T* Get();
    void Put(T&& element);

private:
    std::vector<T> left_, right_;
    std::vector<T> *active_ = &left_, *passive_ = &right_;
};
