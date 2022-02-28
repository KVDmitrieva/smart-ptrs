#pragma once

#include <type_traits>
#include <cstddef>
#include <memory>

template <typename T, size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairEl {
    T value_;
    CompressedPairEl() : value_(T()) {
    }
    template <class Up>
    explicit CompressedPairEl(Up&& val) noexcept : value_(std::forward<Up>(val)) {
    }

    T& GetValue() {
        return value_;
    }
    const T& GetValue() const {
        return value_;
    }
};

template <typename T, size_t I>
struct CompressedPairEl<T, I, true> : public T {
    CompressedPairEl() {
    }
    template <class Up>
    explicit CompressedPairEl(Up&& val) : T(std::forward<Up>(val)) {
    }
    T& GetValue() {
        return *this;
    }
    const T& GetValue() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : private CompressedPairEl<F, 0>, private CompressedPairEl<S, 1> {
    using First = CompressedPairEl<F, 0>;
    using Second = CompressedPairEl<S, 1>;

public:
    CompressedPair() = default;
    template <class Up1, class Up2>
    CompressedPair(Up1&& first, Up2&& second)
        : First(std::forward<Up1>(first)), Second(std::forward<Up2>(second)) {
    }

    F& GetFirst() {
        return First::GetValue();
    }
    const F& GetFirst() const {
        return First::GetValue();
    };

    S& GetSecond() {
        return Second::GetValue();
    };

    const S& GetSecond() const {
        return Second::GetValue();
    };
};
