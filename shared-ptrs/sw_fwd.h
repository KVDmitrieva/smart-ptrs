#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

struct ControlBlock {
    int strong_ = 0;
    int weak_ = 0;
    virtual ~ControlBlock() = default;
    virtual void OnZeroStrong() = 0;
    virtual void OnZeroWeak() = 0;
};