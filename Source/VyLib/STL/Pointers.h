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

    // Wrapper for raw pointer of type T
    // Can be used like a raw pointer
    // Can define additional methods to allow for useful behaviors
    template<typename T>
    class SmartRef
    {
    public:
        // Constructor from pointer
        explicit SmartRef(T* p = nullptr) : 
            m_Ptr(p) 
        {
        }

        // Allow usage like a raw pointer
              T& operator*()        { return *m_Ptr; }
        const T& operator*()  const { return *m_Ptr; }

              T* operator->()       { return  m_Ptr; }
        const T* operator->() const { return  m_Ptr; }

        bool operator==(const SmartRef& other) const { return m_Ptr == other.m_Ptr; }
        bool operator!=(const SmartRef& other) const { return m_Ptr != other.m_Ptr; }

        T* get() const { return m_Ptr; }
        
        // Easy existence check
        explicit operator bool() const { return m_Ptr != nullptr; }

    protected:
        T* m_Ptr;
    };

}