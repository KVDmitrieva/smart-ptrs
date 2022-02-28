#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <utility>

template <typename T>
struct DefaultDeleter {
    void operator()(T* ptr) noexcept {
        delete ptr;
    }
};

template <typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* ptr) noexcept {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
private:
    CompressedPair<T*, Deleter> pair_;

    template <typename U, typename Del>
    friend class UniquePtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : pair_(ptr, Deleter()) {
    }

    template <typename U>
    UniquePtr(T* ptr, U&& deleter) noexcept : pair_(ptr, std::forward<U>(deleter)) {
    }

    UniquePtr(UniquePtr& other) = delete;

    UniquePtr(UniquePtr&& other) noexcept {
        pair_.GetFirst() = static_cast<T*>(other.Release());
        pair_.GetSecond() = std::forward<Deleter>(other.GetDeleter());
    }

    template <typename U>
    UniquePtr(UniquePtr<U>&& other) noexcept {
        pair_.GetFirst() = static_cast<T*>(other.Release());
        //  pair_.GetSecond() = static_cast<Deleter>(std::forward<Del>(other.GetDeleter()));
    }

    template <typename U, typename Del>
    UniquePtr(UniquePtr<U, Del>&& other) noexcept {
        pair_.GetFirst() = static_cast<T*>(other.Release());
        pair_.GetSecond() = static_cast<Deleter>(std::forward<Del>(other.GetDeleter()));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    template <typename U, typename Del>
    UniquePtr& operator=(UniquePtr<U, Del>&& other) noexcept {
        if (pair_.GetFirst() != other.pair_.GetFirst()) {
            Reset(other.Release());
            pair_.GetSecond() = static_cast<Deleter>(std::forward<Del>(other.GetDeleter()));
        }
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t null_ptr) noexcept {
        Reset(null_ptr);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        pair_.GetSecond()(pair_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* tmp = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return tmp;
    }

    void Reset(T* ptr = nullptr) {
        if (pair_.GetFirst() != ptr) {
            T* tmp = pair_.GetFirst();
            pair_.GetFirst() = ptr;
            pair_.GetSecond()(tmp);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(pair_.GetFirst(), other.pair_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }

    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }

    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *pair_.GetFirst();
    }

    T* operator->() const {
        return pair_.GetFirst();
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
private:
    CompressedPair<T*, Deleter> pair_;

    template <typename U, typename Del>
    friend class UniquePtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) noexcept : pair_(ptr, Deleter()) {
    }

    template <typename U>
    UniquePtr(T* ptr, U&& deleter) noexcept : pair_(ptr, std::forward<U>(deleter)) {
    }

    UniquePtr(UniquePtr& other) = delete;

    UniquePtr(UniquePtr&& other) noexcept {
        pair_.GetFirst() = static_cast<T*>(other.Release());
        pair_.GetSecond() = std::forward<Deleter>(other.GetDeleter());
    }

    template <typename U>
    UniquePtr(UniquePtr<U>&& other) noexcept {
        pair_.GetFirst() = static_cast<T*>(other.Release());
        //  pair_.GetSecond() = static_cast<Deleter>(std::forward<Del>(other.GetDeleter()));
    }

    template <typename U, typename Del>
    UniquePtr(UniquePtr<U, Del>&& other) noexcept {
        pair_.GetFirst() = static_cast<T*>(other.Release());
        pair_.GetSecond() = static_cast<Deleter>(std::forward<Del>(other.GetDeleter()));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    template <typename U, typename Del>
    UniquePtr& operator=(UniquePtr<U, Del>&& other) noexcept {
        if (pair_.GetFirst() != other.pair_.GetFirst()) {
            Reset(other.Release());
            pair_.GetSecond() = static_cast<Deleter>(std::forward<Del>(other.GetDeleter()));
        }
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t null_ptr) noexcept {
        Reset(null_ptr);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        pair_.GetSecond()(pair_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* tmp = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return tmp;
    }

    void Reset(T* ptr = nullptr) {
        if (pair_.GetFirst() != ptr) {
            T* tmp = pair_.GetFirst();
            pair_.GetFirst() = ptr;
            pair_.GetSecond()(tmp);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(pair_.GetFirst(), other.pair_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }

    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }

    explicit operator bool() const {
        return pair_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *pair_.GetFirst();
    }

    T* operator->() const {
        return pair_.GetFirst();
    }

    std::add_lvalue_reference_t<T> operator[](size_t i) {
        return pair_.GetFirst()[i];
    }

    std::add_lvalue_reference_t<T> operator[](size_t i) const {
        return pair_.GetFirst()[i];
    }
};
