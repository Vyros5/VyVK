#pragma once

#include <memory>

namespace Vy
{

    // --------------------------------------------------------------------------------------------

    template<typename T>
    using Shared = std::shared_ptr<T>;

    template<typename T, typename... Args>
    constexpr Shared<T> MakeShared(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
    template<typename T>
    constexpr Shared<T> MakeShared(const T& t) 
    {
        return std::make_shared<T>(t);
    }

    // --------------------------------------------------------------------------------------------

    template<typename T>
    using Unique = std::unique_ptr<T>;

    template<typename T, typename... Args>
    constexpr Unique<T> MakeUnique(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    // --------------------------------------------------------------------------------------------

    template <typename T>
    using WeakRef = std::weak_ptr<T>;

    // --------------------------------------------------------------------------------------------

    // https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast.html

    template<typename D, typename T>
    Unique<D> DynamicCast(Unique<T> ptr)
    {
        if(auto ret = dynamic_cast<D*>(ptr.get()))
        {
            ptr.release();
            return Unique<D>(ret);
        }

        return nullptr;
    }

    // --------------------------------------------------------------------------------------------



}