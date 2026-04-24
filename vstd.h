#pragma once

#include <concepts>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <utility>

namespace vstd {

template <typename Container, typename Value>
concept ContainsComparable = requires(const Container &container, const Value &value) {
    { container.find(value) };
    { container.end() };
};

template <typename Queue>
concept QueueLike = requires(Queue &queue) {
    { queue.front() };
    { queue.pop() } -> std::same_as<void>;
};

template <typename Container, typename Value>
requires ContainsComparable<Container, Value>
[[nodiscard]] bool ctn(const Container &container, const Value &value) {
    return container.find(value) != container.end();
}

inline std::mt19937 &rng() {
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}

inline int rand(int max_exclusive) {
    if (max_exclusive <= 0) {
        return 0;
    }
    std::uniform_int_distribution<int> dist(0, max_exclusive - 1);
    return dist(rng());
}

inline int rand(int min_inclusive, int max_inclusive) {
    if (max_inclusive < min_inclusive) {
        return min_inclusive;
    }
    std::uniform_int_distribution<int> dist(min_inclusive, max_inclusive);
    return dist(rng());
}

template <typename T>
std::string str(const T &value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

template <QueueLike Queue>
auto pop(Queue &queue) {
    auto front = queue.front();
    queue.pop();
    return front;
}

} // namespace vstd
