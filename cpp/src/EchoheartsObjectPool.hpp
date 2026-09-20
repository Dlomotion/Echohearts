#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Echohearts::Core {

// Lightweight single-threaded pool for ordinary C++ objects.
// Do not use this for Unreal Engine UObjects or AActors; UE owns those lifetimes.
template <typename T>
class ObjectPool final {
    static_assert(!std::is_pointer_v<T>, "ObjectPool<T> expects an object type, not a pointer type");
    static_assert(std::is_default_constructible_v<T>, "ObjectPool<T> requires a default-constructible type");

public:
    using ResetFunction = std::function<void(T&)>;

    explicit ObjectPool(std::size_t initialCapacity = 0, ResetFunction reset = {})
        : reset_(std::move(reset))
    {
        Reserve(initialCapacity);
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;

    [[nodiscard]] T* Acquire()
    {
        if (free_.empty()) {
            Grow(1);
        }

        T* object = free_.back();
        free_.pop_back();

        const auto [_, inserted] = inUse_.insert(object);
        if (!inserted) {
            throw std::logic_error("ObjectPool internal state error: object already marked in use");
        }
        return object;
    }

    [[nodiscard]] bool Release(T* object)
    {
        if (object == nullptr || owned_.find(object) == owned_.end()) {
            return false;
        }

        if (inUse_.erase(object) != 1U) {
            return false;
        }

        if (reset_) {
            reset_(*object);
        }
        free_.push_back(object);
        return true;
    }

    void ReleaseAll()
    {
        for (T* object : inUse_) {
            if (reset_) {
                reset_(*object);
            }
            free_.push_back(object);
        }
        inUse_.clear();
    }

    void Reserve(std::size_t capacity)
    {
        if (capacity > storage_.size()) {
            Grow(capacity - storage_.size());
        }
    }

    [[nodiscard]] bool Owns(const T* object) const noexcept
    {
        return object != nullptr && owned_.find(const_cast<T*>(object)) != owned_.end();
    }

    [[nodiscard]] std::size_t Capacity() const noexcept { return storage_.size(); }
    [[nodiscard]] std::size_t Available() const noexcept { return free_.size(); }
    [[nodiscard]] std::size_t InUse() const noexcept { return inUse_.size(); }

private:
    void Grow(std::size_t count)
    {
        storage_.reserve(storage_.size() + count);
        free_.reserve(free_.size() + count);
        owned_.reserve(owned_.size() + count);
        inUse_.reserve(inUse_.size() + count);

        for (std::size_t i = 0; i < count; ++i) {
            auto object = std::make_unique<T>();
            T* raw = object.get();
            storage_.push_back(std::move(object));
            owned_.insert(raw);
            free_.push_back(raw);
        }
    }

    std::vector<std::unique_ptr<T>> storage_;
    std::vector<T*> free_;
    std::unordered_set<T*> owned_;
    std::unordered_set<T*> inUse_;
    ResetFunction reset_;
};

} // namespace Echohearts::Core
