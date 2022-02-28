#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t


template <typename T>
struct ControlBlockPointer : ControlBlock {
    T* ptr_ = nullptr;
    ControlBlockPointer(T* ptr) : ptr_(ptr){};
    ~ControlBlockPointer() override = default;
    void OnZeroStrong() override {
        delete ptr_;
        ptr_ = nullptr;
    }
    void OnZeroWeak() override {
        delete this;
    }
};

template <typename T>
struct ControlBlockMakeShared : ControlBlock {
    std::aligned_storage_t<sizeof(T), alignof(T)> storage;

    template <typename... Args>
    ControlBlockMakeShared(Args&&... args) {
        new (&storage) T{std::forward<Args>(args)...};
    }
    ~ControlBlockMakeShared() override = default;

    void OnZeroStrong() override {
        GetRawPtr()->~T();
    }
    void OnZeroWeak() override {
        delete this;
    }

    T* GetRawPtr() {
        return reinterpret_cast<T*>(&storage);
    }
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
private:
    ControlBlock* state_{nullptr};
    T* ptr_{nullptr};

    template <typename Y>
    friend class SharedPtr;

    friend class WeakPtr<T>;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() noexcept {
    }
    SharedPtr(std::nullptr_t) noexcept {
    }

    explicit SharedPtr(T* ptr) noexcept {
        state_ = new ControlBlockPointer<T>(ptr);
        ptr_ = ptr;
        ++state_->strong_;
    }
    template <typename Y>
    explicit SharedPtr(Y* ptr) noexcept {
        state_ = new ControlBlockPointer<Y>(ptr);
        ptr_ = ptr;
        ++state_->strong_;
    }

    SharedPtr(ControlBlockMakeShared<T>* block, T* ptr) : state_(block), ptr_(ptr) {
    }

    SharedPtr(const SharedPtr& other) {
        state_ = other.state_;
        ptr_ = other.ptr_;
        if (state_) {
            ++state_->strong_;
        }
    }
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) {
        state_ = other.state_;
        ptr_ = other.ptr_;
        if (state_) {
            ++state_->strong_;
        }
    }

    SharedPtr(SharedPtr&& other) {
        state_ = other.state_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        other.state_ = nullptr;
    }
    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) {
        state_ = other.state_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        other.state_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        state_ = other.state_;
        if (state_) {
            state_->strong_++;
        }
        ptr_ = ptr;
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        ptr_ = other.ptr_;
        state_ = other.state_;
        ++state_->strong_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (state_ != other.state_) {
            Free();
            state_ = other.state_;
            ptr_ = other.ptr_;
            if (state_) {
                ++state_->strong_;
            }
        }
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) {
        if (other.state_ != state_) {
            std::swap(state_, other.state_);
            std::swap(ptr_, other.ptr_);
            other.Free();
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() noexcept {
        Free();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        Free();
    }
    template <typename Y>
    void Reset(Y* ptr) {
        Free();
        state_ = new ControlBlockPointer<Y>(ptr);
        ptr_ = ptr;
        state_->strong_ = 1;
    }
    void Swap(SharedPtr& other) {
        std::swap(state_, other.state_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        return (state_) ? state_->strong_ : 0;
    }
    explicit operator bool() const {
        return state_ != nullptr && ptr_ != nullptr;
    }

private:
    void Free() {
        if (state_) {
            --state_->strong_;
            if (state_->strong_ == 0) {
                state_->OnZeroStrong();
                if (state_->weak_ == 0) {
                    state_->OnZeroWeak();
                }
            }
        }
        state_ = nullptr;
        ptr_ = nullptr;
    }
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new ControlBlockMakeShared<T>(std::forward<Args>(args)...);
    ++block->strong_;
    return SharedPtr<T>(block, block->GetRawPtr());
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
