#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
private:
    ControlBlock* state_{nullptr};
    T* ptr_{nullptr};

    friend class SharedPtr<T>;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() = default;

    WeakPtr(const WeakPtr& other) {
        ptr_ = other.ptr_;
        state_ = other.state_;
        if (state_) {
            ++state_->weak_;
        }
    }
    WeakPtr(WeakPtr&& other) {
        ptr_ = other.ptr_;
        state_ = other.state_;
        other.Reset();
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        ptr_ = other.ptr_;
        state_ = other.state_;
        if (state_) {
            ++state_->weak_;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (other.state_ != state_) {
            ptr_ = other.ptr_;
            state_ = other.state_;
            if (state_) {
                ++state_->weak_;
            }
        }
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (other.state_ != state_) {
            std::swap(ptr_, other.ptr_);
            std::swap(other.state_, state_);
            other.Reset();
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (state_) {
            --state_->weak_;
            if (state_->strong_ + state_->weak_ == 0) {
                state_->OnZeroWeak();
            }
        }
        state_ = nullptr;
        ptr_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(state_, other.state_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        return (state_) ? state_->strong_ : 0;
    }
    bool Expired() const {
        return state_ == nullptr || state_->strong_ == 0;
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }
};
