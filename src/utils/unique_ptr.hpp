#pragma once

#include <utility>

namespace utils
{

template <typename T>
struct UniquePtr final
{
    constexpr UniquePtr()
        : mPtr(nullptr)
    {
    }

    constexpr UniquePtr(T* ptr)
        : mPtr(ptr)
    {
    }

    constexpr ~UniquePtr()
    {
        reset();
    }

    constexpr UniquePtr(const UniquePtr&)
#if __cpp_deleted_function >= 202403L
        = delete("copy of UniquePtr is not allowed")
#endif
        ;

    constexpr UniquePtr(UniquePtr&& other)
        : mPtr(other.mPtr)
    {
        other.mPtr = nullptr;
    }

    template <typename U>
    constexpr UniquePtr(UniquePtr<U>&& other)
        : mPtr(static_cast<T*>(other.mPtr))
    {
        other.mPtr = nullptr;
    }

    constexpr UniquePtr& operator=(const UniquePtr&)
#if __cpp_deleted_function >= 202403L
        = delete("copy of UniquePtr is not allowed")
#endif
        ;

    constexpr UniquePtr& operator=(UniquePtr&& other)
    {
        reset();
        mPtr = other.mPtr;
        other.mPtr = nullptr;
        return *this;
    }

    template <typename U>
    constexpr UniquePtr& operator=(UniquePtr<U>&& other)
    {
        reset();
        mPtr = static_cast<T*>(other.mPtr);
        other.mPtr = nullptr;
        return *this;
    }

    constexpr auto release()
    {
        auto ptr = mPtr;
        mPtr = nullptr;
        return ptr;
    }

    constexpr void reset()
    {
        if (mPtr)
        {
            delete mPtr;
            mPtr = nullptr;
        }
    }

    constexpr operator bool() const { return mPtr != nullptr; }

    constexpr auto operator->() const { return mPtr; }
    constexpr auto operator->()       { return mPtr; }

    constexpr auto& operator*() const { return *mPtr; }
    constexpr auto& operator*()       { return *mPtr; }

    constexpr auto get() const { return mPtr; }
    constexpr auto get()       { return mPtr; }

    constexpr bool operator==(const T* other) const
    {
        return mPtr == other;
    }

private:
    template <typename> friend struct UniquePtr;
    T* mPtr;
};

template <typename T, typename ...Args>
UniquePtr<T> makeUnique(Args&&... args)
{
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace utils
