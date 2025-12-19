#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/STL/String.h>
#include <VyLib/STL/Pointers.h>
#include <VyLib/STL/Containers.h>
#include <VyLib/Core/Defines.h>
#include <VyLib/Core/Assert.h>













// #include <algorithm>

// namespace Vy
// {
//     namespace Memory
//     {
//         // Could abstract this in some way (by defining those as function pointers) to allow for custom allocators
//         // For now, I'll leave it as it is

//         /**
//          * @brief allocate a chunk of memory of a given size.
//          *
//          * Uses default `std::malloc`. It is here as a placeholder for future custom global allocators.
//          *
//          * @param size The size of the memory to allocate.
//          * @return A pointer to the allocated memory.
//          */
//         /*VY_API*/ void *allocate(size_t size);

//         /**
//          * @brief deallocate a chunk of memory
//          *
//          * Uses default `std::free`. It is here as a placeholder for future custom global allocators.
//          *
//          * @param pPtr A pointer to the memory to deallocate.
//          */
//         /*VY_API*/ void deallocate(void *pPtr);

//         /**
//          * @brief allocate a chunk of memory of a given size with a given alignment.
//          *
//          * Uses the default platform-specific aligned allocation. It is here as a placeholder for future custom global
//          * allocators.
//          *
//          * @param size The size of the memory to allocate.
//          * @param alignment The alignment of the memory to allocate.
//          * @return A pointer to the allocated memory.
//          */
//         /*VY_API*/ void *allocateAligned(size_t size, size_t alignment);

//         /**
//          * @brief deallocate a chunk of memory with a given alignment.
//          *
//          * Uses the default platform-specific aligned deallocation. It is here as a placeholder for future custom global
//          * allocators.
//          *
//          * @param pPtr A pointer to the memory to deallocate.
//          * @param alignment The alignment of the memory to deallocate.
//          */
//         /*VY_API*/ void deallocateAligned(void *pPtr);

//         /**
//          * @brief Copy a chunk of memory from one location to another.
//          *
//          * Uses the default `::memcpy()`. It is here as a placeholder for future custom global memory management.
//          *
//          * @param pDst A pointer to the destination memory.
//          * @param pSrc A pointer to the source memory.
//          * @param size The size of the memory to copy, in bytes.
//          */
//         /*VY_API*/ void *forwardCopy(void *pDst, const void *pSrc, size_t size);

//         /**
//          * @brief Copy a chunk of memory from one location to another in reverse order.
//          *
//          * Uses the default `::memmove()`. It is here as a placeholder for future custom global memory management.
//          *
//          * @param pDst A pointer to the destination memory.
//          * @param pSrc A pointer to the source memory.
//          * @param size The size of the memory to copy, in bytes.
//          */
//         /*VY_API*/ void* backwardCopy(void *pDst, const void *pSrc, size_t size);

//         /**
//          * @brief Copy a range of elements from one iterator to another.
//          *
//          * Uses the default `std::copy()`. It is here as a placeholder for future custom global memory management.
//          *
//          * @param pDst An iterator pointing to the destination memory.
//          * @param itBegin An iterator pointing to the beginning of the source memory.
//          * @param itEnd   An iterator pointing to the end of the source memory.
//          */
//         template <typename It1, typename It2> 
//         constexpr auto forwardCopy(It1 pDst, It2 itBegin, It2 itEnd)
//         {
//             return std::copy(itBegin, itEnd, pDst);
//         }

//         /**
//          * @brief Copy a range of elements from one iterator to another in reverse order.
//          *
//          * Uses the default `std::copy_backward()`. It is here as a placeholder for future custom global memory management.
//          *
//          * @param pDst An iterator pointing to the destination memory.
//          * @param itBegin An iterator pointing to the beginning of the source memory.
//          * @param itEnd   An iterator pointing to the end of the source memory.
//          */
//         template <typename It1, typename It2> 
//         constexpr auto backwardCopy(It1 pDst, It2 itBegin, It2 itEnd)
//         {
//             return std::copy_backward(itBegin, itEnd, pDst);
//         }

//         /**
//          * @brief Move a chunk of memory from one location to another.
//          *
//          * Uses the default `std::move()`. It is here as a placeholder for future custom global memory management.
//          *
//          * @param pDst An iterator pointing to the destination memory.
//          * @param itBegin An iterator pointing to the beginning of the source memory.
//          * @param itEnd   An iterator pointing to the end of the source memory.
//          */
//         template <typename It1, typename It2> constexpr auto ForwardMove(It1 pDst, It2 itBegin, It2 itEnd)
//         {
//             return std::move(itBegin, itEnd, pDst);
//         }

//         /**
//          * @brief Move a chunk of memory from one location to another in reverse order.
//          *
//          * Uses the default `std::move_backward()`. It is here as a placeholder for future custom global memory management.
//          *
//          * @param pDst An iterator pointing to the destination memory.
//          * @param itBegin An iterator pointing to the beginning of the source memory.
//          * @param itEnd   An iterator pointing to the end of the source memory.
//          */
//         template <typename It1, typename It2> constexpr auto BackwardMove(It1 pDst, It2 itBegin, It2 itEnd)
//         {
//             return std::move_backward(itBegin, itEnd, pDst);
//         }

//         /**
//          * @brief Check if an adress pointer is aligned to a specific alignment.
//          *
//          * @param pPtr The adress that will be checked for alignment.
//          * @param alignment The required alignment.
//          */
//         bool isAligned(const void *pPtr, size_t alignment);

//         /**
//          * @brief Check if an adress pointer is aligned to a specific alignment.
//          *
//          * @param pPtr The adress that will be checked for alignment.
//          * @param alignment The required alignment.
//          */
//         bool isAligned(size_t address, size_t alignment);

//         /**
//          * @brief A custom allocator that uses a custom size_type (usually `u32`) for indexing, compatible with STL.
//          *
//          * This allocator is intended for environments or applications where the maximum container size never exceeds 2^32,
//          * making 32-bit indices (`u32`) sufficient. By using a smaller index type, it can offer performance benefits in tight
//          * loops and reduce cache pressure, particularly when managing a large number of small containers or indices.
//          *
//          * @tparam T The type of object to allocate.
//          *
//          * @note
//          * - This allocator is stateless and always compares equal.
//          *
//          * - It does not provide `construct()` or `destroy()` methods explicitly
//          *   because, in C++17 and later, `std::allocator_traits` can handle
//          *   construction and destruction via placement-new and direct destructor calls.
//          */
//         template <typename T> class STLAllocator
//         {
//         public:
//             using value_type = T;
//             using pointer = T*;
//             using const_pointer = const T*;
//             using size_type = usize;
//             using difference_type = idiff;

//             template <typename U> struct rebind
//             {
//                 using other = STLAllocator<U>;
//             };

//             STLAllocator() = default;

//             template <typename U> STLAllocator(const STLAllocator<U> &)
//             {
//             }

//             pointer allocate(size_type p_N)
//             {
//                 return static_cast<pointer>(allocate(p_N * sizeof(T)));
//             }

//             void deallocate(pointer pPtr, size_type)
//             {
//                 deallocate(pPtr);
//             }

//             bool operator==(const STLAllocator &) const
//             {
//                 return true;
//             }
//             bool operator!=(const STLAllocator &) const
//             {
//                 return false;
//             }
//         };

//         /**
//          * @brief construct an object of type `T` in the given memory location.
//          *
//          * @note This function does not allocate memory. It only calls the constructor of the object.
//          *
//          * @param pPtr A pointer to the memory location where the object should be constructed.
//          * @param args The arguments to pass to the constructor of `T`.
//          * @return A pointer to the constructed object.
//          */
//         template <typename T, typename... Args> 
//         T* construct(T* pPtr, Args &&...args)
//         {
//             VY_ASSERT(isAligned(pPtr, alignof(T)), "[MEMORY] The address used to construct an object is not correctly aligned to its alignment");

//             return std::launder(::new (pPtr) T(std::forward<Args>(args)...));
//         }

//         /**
//          * @brief Destroy an object of type `T` in the given memory location.
//          *
//          * @note This function does not deallocate the memory. It only calls the destructor of the object.
//          *
//          * @param pPtr A pointer to the memory location where the object should be destroyed.
//          */
//         template <typename T> 
//         void destruct(T* pPtr)
//         {
//             pPtr->~T();
//         }

//         /**
//          * @brief construct an object of type `T` in the memory location an iterator points to.
//          *
//          * Useful when dealing with iterators that could be pointers themselves.
//          *
//          * @note This function does not allocate memory. It only calls the constructor of the object.
//          *
//          * @param it   An iterator pointing to the memory location where the object should be constructed.
//          * @param args The arguments to pass to the constructor of `T`.
//          * @return A pointer to the constructed object.
//          */
//         template <typename It, typename... Args> 
//         auto constructFromIterator(const It it, Args &&...args)
//         {
//             if constexpr (std::is_pointer_v<It>)
//             {
//                 return construct(it, std::forward<Args>(args)...);
//             }
//             else
//             {
//                 return construct(&*it, std::forward<Args>(args)...);
//             }
//         }

//         /**
//          * @brief Destroy an object of type `T` in the memory location an iterator points to.
//          *
//          * Useful when dealing with iterators that could be pointers themselves.
//          *
//          * @note This function does not deallocate the memory. It only calls the destructor of the object.
//          *
//          * @param it An iterator pointing to the memory location where the object should be destroyed.
//          */
//         template <typename It> 
//         void destructFromIterator(const It it)
//         {
//             if constexpr (std::is_pointer_v<It>)
//             {
//                 destruct(it);
//             }
//             else
//             {
//                 destruct(&*it);
//             }
//         }

//         /**
//          * @brief construct a range of objects of type `T` given some constructor arguments.
//          *
//          * @note This function does not allocate memory. It only calls the constructor of the object.
//          *
//          * @param itBegin An iterator pointing to the beginning of the range where the objects should be constructed.
//          * @param itEnd   An iterator pointing to the end of the range where the objects should be constructed.
//          * @param args The arguments to pass to the constructor of `T`.
//          */
//         template <typename It, typename... Args> 
//         void constructRange(const It itBegin, const It itEnd, const Args &...args)
//         {
//             for (auto it = itBegin; it != itEnd; ++it)
//             {
//                 constructFromIterator(it, args...);
//             }
//         }

//         /**
//          * @brief construct a range of objects of type `T` by copying from another range.
//          *
//          * @note This function does not allocate memory. It only calls the constructor of the object.
//          *
//          * @param pDst An iterator pointing to the beginning of the destination range where the objects should be constructed.
//          * @param itBegin An iterator pointing to the beginning of the source range where the objects should be copied from.
//          * @param itEnd   An iterator pointing to the end of the source range where the objects should be copied from.
//          */
//         template <typename It1, typename It2> 
//         void constructRangeCopy(It1 pDst, const It2 itBegin, const It2 itEnd)
//         {
//             for (auto it = itBegin; it != itEnd; ++it, ++pDst)
//             {
//                 constructFromIterator(pDst, *it);
//             }
//         }

//         /**
//          * @brief construct a range of objects of type `T` by moving from another range.
//          *
//          * @note This function does not allocate memory. It only calls the constructor of the object.
//          *
//          * @param pDst An iterator pointing to the beginning of the destination range where the objects should be constructed.
//          * @param itBegin An iterator pointing to the beginning of the source range where the objects should be moved from.
//          * @param itEnd   An iterator pointing to the end of the source range where the objects should be moved from.
//          */
//         template <typename It1, typename It2> 
//         void constructRangeMove(It1 pDst, const It2 itBegin, const It2 itEnd)
//         {
//             for (auto it = itBegin; it != itEnd; ++it, ++pDst)
//             {
//                 constructFromIterator(pDst, std::move(*it));
//             }
//         }

//         /**
//          * @brief Destroy a range of objects of type `T`.
//          *
//          * @note This function does not deallocate the memory. It only calls the destructor of the object.
//          *
//          * @param itBegin An iterator pointing to the beginning of the range where the objects should be destroyed.
//          * @param itEnd   An iterator pointing to the end of the range where the objects should be destroyed.
//          */
//         template <typename It> 
//         void destructRange(const It itBegin, const It itEnd)
//         {
//             for (auto it = itBegin; it != itEnd; ++it)
//             {
//                 destructFromIterator(it);
//             }
//         }
//     }


//     /**
//      * @brief A raw storage class mainly used to allow the deferred creation and destruction of objects using a fixed size
//      * buffer with no heap allocations.
//      *
//      * This is useful when having a class with no default constructor (having strict initialization requirements) that is
//      * being used in another class for which the construction requirements of the original class may not be met at the time
//      * of construction.
//      *
//      * This class is trivially copyable and movable. Be cautios when using it with complex types that have non-trivial copy
//      * or move constructors.
//      *
//      * @tparam TSize The size of the local allocation.
//      * @tparam TAlignment The alignment of the local allocation, defaults to the alignment of a pointer.
//      */
//     template <
//         USize TSize, 
//         USize TAlignment = alignof(std::max_align_t)
//     > 
//     class RawStorage
//     {
//     public:
//         constexpr RawStorage() = default;

//         /**
//          * @brief construct a new object of type `T` in the local buffer.
//          *
//          * Calling `construct()` on top of an existing object will cause undefined behavior. The object of type `T` needs to
//          * fit in the local buffer and have an alignment that is compatible with the local buffer.
//          *
//          * If your type is trivially constructible, you dont need to call this function.
//          *
//          * @tparam T The type of the object to create.
//          * @param args The arguments to pass to the constructor of `T`.
//          * @return A pointer to the newly created object.
//          */
//         template <typename T, typename... Args> 
//         constexpr T* construct(Args &&... args)
//         {
//             static_assert(sizeof(T)  <= TSize,      "Object does not fit in the local buffer");
//             static_assert(alignof(T) <= TAlignment, "Object has incompatible alignment");

//             return Memory::construct(get<T>(), std::forward<Args>(args)...);
//         }

//         /**
//          * @brief Destroy the object in the local buffer.
//          *
//          * Calling `destruct()` on top of an already destroyed object/uninitialized memory, or calling `destruct()` with a
//          * different type `T` will cause undefined behavior.
//          *
//          * If `T`is trivially destructible, this function will do nothing.
//          *
//          * This function is declared as const to follow the standard pattern where a pointer to const object can be
//          * destroyed.
//          *
//          * @tparam T The type of the object to destroy.
//          */
//         template <typename T> constexpr void destruct() const
//         {
//             if constexpr (!std::is_trivially_destructible_v<T>)
//             {
//                 Memory::destruct(get<T>());
//             }
//         }

//         /**
//          * @brief get a pointer to the object in the local buffer.
//          *
//          * Calling `get()` with a different type `T` will cause undefined behavior (uses reinterpret_cast under the hood).
//          *
//          * @tparam T The type of the object to get.
//          * @return A pointer to the object in the local buffer.
//          */
//         template <typename T> constexpr const T* get() const
//         {
//             return reinterpret_cast<const T*>(m_Data);
//         }

//         /**
//          * @brief get a pointer to the object in the local buffer.
//          *
//          * Calling `get()` with a different type `T` will cause undefined behavior (uses reinterpret_cast under the hood).
//          *
//          * @tparam T The type of the object to get.
//          * @return A pointer to the object in the local buffer.
//          */
//         template <typename T> constexpr T* get()
//         {
//             return reinterpret_cast<T*>(m_Data);
//         }

//     private:
//         alignas(TAlignment) std::byte m_Data[TSize];
//     };



//     /**
//      * @brief A class that wraps a RawStorage object and provides a more user-friendly interface for creating and destroying
//      * objects.
//      *
//      * It is safer to use as it emposes more restrictions on its usage. It will adapt to the size and alignment of
//      * the specified type, and the copy and move constructors and assignment operators will be generated based on the type
//      * T.
//      *
//      * To avoid a boolean check overhead and grant the user more constrol over the destruction of the object, the `T`'s
//      * destructor will not be called automatically when the Storage object goes out of scope.
//      *
//      * @tparam T The type of object to store.
//      */
//     template <typename T> class Storage
//     {
//     public:
//         constexpr Storage() = default;

//         constexpr Storage(const Storage &other)
//             requires std::copy_constructible<T>
//         {
//             m_Storage.template construct<T>(*other.get());
//         }

//         constexpr Storage(Storage &&other)
//             requires std::move_constructible<T>
//         {
//             m_Storage.template construct<T>(std::move(*other.get()));
//         }

//         constexpr Storage& operator=(const Storage &other)
//             requires std::is_copy_assignable_v<T>
//         {
//             if (this != &other)
//             {
//                 *get() = *other.get();
//             }

//             return *this;
//         }

//         constexpr Storage &operator=(Storage &&other)
//             requires std::is_move_assignable_v<T>
//         {
//             if (this != &other)
//             {
//                 *get() = std::move(*other.get());
//             }

//             return *this;
//         }

//         template <typename... Args>
//             requires(!std::same_as<Storage, std::decay_t<Args>> && ...)
//         constexpr Storage(Args&&... args)
//         {
//             m_Storage.template construct<T>(std::forward<Args>(args)...);
//         }

//         /**
//          * @brief construct a new object of type `T` in the local buffer.
//          *
//          * Calling `construct()` on top of an existing object will cause undefined behavior.
//          *
//          * @param args The arguments to pass to the constructor of `T`.
//          * @return A pointer to the newly created object.
//          */
//         template <typename... Args> constexpr T* construct(Args &&...args)
//         {
//             return m_Storage.template construct<T>(std::forward<Args>(args)...);
//         }

//         /**
//          * @brief destruct the object in the local buffer.
//          *
//          * Calling `destruct()` on top of an already destroyed object/uninitialized memory, or calling `destruct()` with a
//          * different type `T` will cause undefined behavior.
//          *
//          * This function is declared as const to follow the standard pattern where a pointer to const object can be
//          * destroyed.
//          */
//         constexpr void destruct() const
//         {
//             m_Storage.template destruct<T>();
//         }

//         constexpr const T* get() const
//         {
//             return m_Storage.template get<T>();
//         }

//         constexpr T* get()
//         {
//             return m_Storage.template get<T>();
//         }

//         constexpr const T* operator->() const
//         {
//             return get();
//         }

//         constexpr T* operator->()
//         {
//             return get();
//         }

//         constexpr const T& operator*() const
//         {
//             return *get();
//         }

//         constexpr T& operator*()
//         {
//             return *get();
//         }

//     private:
//         RawStorage<sizeof(T), alignof(T)> m_Storage;
//     };









//     /**
//      * @brief A result class that can hold either a value of type `T` or an error message of type ErrorType.
//      *
//      * This class is meant to be used in functions that can fail and return an error message, or succeed and return a value.
//      * The main difference between this class and `std::optional` is that this class explicitly holds an error if the result
//      * could not be computed. It is meant to make my life easier to be honest.
//      *
//      * Using the default constructor will create an uninitialized `Result`. Make sure to instantiate it with either `Ok` or
//      * `Error` before using it.
//      *
//      * @tparam T The type of the value that can be held.
//      * @tparam ErrorType The type of the error message that can be held.
//      */
//     template <typename T = void, typename ErrorType = const char*> 
//     class Result
//     {
//         using Flags = U8;

//         enum FlagBits : Flags
//         {
//             RESULT_FLAG_OK      = 1 << 0,
//             RESULT_FLAG_ENGAGED = 1 << 1
//         };

//     public:
//         /**
//          * @brief construct a `Result` object with a value of type `T`.
//          *
//          * @param args The arguments to pass to the constructor of `T`.
//          */
//         template <typename... ValueArgs> 
//         static Result Ok(ValueArgs&&... args)
//         {
//             Result result{};
//             {
//                 result.m_Flags = RESULT_FLAG_ENGAGED | RESULT_FLAG_OK;
//                 result.m_Value.construct(std::forward<ValueArgs>(args)...);
//             }

//             return result;
//         }

//         /**
//          * @brief construct a `Result` object with an error message of type `ErrorType`.
//          *
//          * @param args The arguments to pass to the constructor of `ErrorType`.
//          */
//         template <typename... ErrorArgs> 
//         static Result Error(ErrorArgs&&... args)
//         {
//             Result result{};
//             {
//                 result.m_Flags = RESULT_FLAG_ENGAGED;
//                 result.m_Error.construct(std::forward<ErrorArgs>(args)...);
//             }

//             return result;
//         }

//         Result(const Result& other)
//             requires(std::copy_constructible<T> && std::copy_constructible<ErrorType>)
//             : m_Flags(other.m_Flags)
//         {
//             if (!checkFlag(RESULT_FLAG_ENGAGED))
//             {
//                 return;
//             }
//             if (checkFlag(RESULT_FLAG_OK))
//             {
//                 m_Value.construct(*other.m_Value.get());
//             }
//             else
//             {
//                 m_Error.construct(*other.m_Error.get());
//             }
//         }

//         Result(Result&& other)
//             requires(std::move_constructible<T> && std::move_constructible<ErrorType>)
//             : m_Flags(other.m_Flags)
//         {
//             if (!checkFlag(RESULT_FLAG_ENGAGED))
//             {
//                 return;
//             }
//             if (checkFlag(RESULT_FLAG_OK))
//             {
//                 m_Value.construct(std::move(*other.m_Value.get()));
//             }
//             else
//             {
//                 m_Error.construct(std::move(*other.m_Error.get()));
//             }
//         }

//         Result& operator=(const Result& other)
//             requires(std::is_copy_assignable_v<T> && std::is_copy_assignable_v<ErrorType>)
//         {
//             if (this == &other)
//             {
//                 return *this;
//             }

//             destroy();
//             m_Flags = other.m_Flags;

//             if (!checkFlag(RESULT_FLAG_ENGAGED))
//             {
//                 return *this;
//             }
//             if (checkFlag(RESULT_FLAG_OK))
//             {
//                 m_Value.construct(*other.m_Value.get());
//             }
//             else
//             {
//                 m_Error.construct(*other.m_Error.get());
//             }

//             return *this;
//         }

//         Result& operator=(Result&& other)
//             requires(std::is_move_assignable_v<T> && std::is_move_assignable_v<ErrorType>)
//         {
//             if (this == &other)
//             {
//                 return *this;
//             }

//             destroy();
//             m_Flags = other.m_Flags;

//             if (!checkFlag(RESULT_FLAG_ENGAGED))
//             {
//                 return *this;
//             }
//             if (checkFlag(RESULT_FLAG_OK))
//             {
//                 m_Value.construct(std::move(*other.m_Value.get()));
//             }
//             else
//             {
//                 m_Error.construct(std::move(*other.m_Error.get()));
//             }

//             return *this;
//         }

//         ~Result()
//         {
//             destroy();
//         }

//         /**
//          * @brief Check if the result is valid.
//          *
//          */
//         bool isOk() const
//         {
//             return checkFlag(RESULT_FLAG_ENGAGED) && checkFlag(RESULT_FLAG_OK);
//         }

//         bool isError() const
//         {
//             return checkFlag(RESULT_FLAG_ENGAGED) && !checkFlag(RESULT_FLAG_OK);
//         }

//         /**
//          * @brief get the value of the result.
//          *
//          * If the result is not valid, this will cause undefined behavior.
//          *
//          */
//         const T& value() const
//         {
//             VY_ASSERT(isOk(), "[RESULT] Result is not Ok");
//             return *m_Value.get();
//         }

//         /**
//          * @brief get the value of the result.
//          *
//          * If the result is not valid, this will cause undefined behavior.
//          *
//          */
//         T& value()
//         {
//             VY_ASSERT(isOk(), "[RESULT] Result is not Ok");

//             return *m_Value.get();
//         }

//         /**
//          * @brief get the error message of the result.
//          *
//          * If the result is valid, this will cause undefined behavior.
//          *
//          */
//         const ErrorType& error() const
//         {
//             VY_ASSERT(isError(), "[RESULT] Result is not an error");

//             return *m_Error.get();
//         }

//         const T* operator->() const
//         {
//             return &value();
//         }

//         T* operator->()
//         {
//             return &value();
//         }

//         const T& operator*() const
//         {
//             return value();
//         }

//         T& operator*()
//         {
//             return value();
//         }

//         operator bool() const
//         {
//             return isOk();
//         }

//     private:
//         Result() = default;

//         bool checkFlag(const FlagBits flag) const
//         {
//             return m_Flags & flag;
//         }

//         void destroy()
//         {
//             if (!checkFlag(RESULT_FLAG_ENGAGED))
//             {
//                 return;
//             }

//             if (checkFlag(RESULT_FLAG_OK))
//             {
//                 m_Value.destruct();
//             }
//             else
//             {
//                 m_Error.destruct();
//             }
//         }

//         union 
//         {
//             Storage<T>         m_Value;
//             Storage<ErrorType> m_Error;
//         };
        
//         Flags m_Flags = 0;
//     };



//     /**
//      * @brief A specialization of `Result` that does not hold a value.
//      *
//      * This class is meant to be used in functions that can fail and return an error message, or succeed and return nothing.
//      * The main difference between this class and `std::optional` is that this class explicitly holds an error if the result
//      * could not be computed. It is meant to make my life easier to be honest.
//      *
//      * Using the default constructor will create an uninitialized `Result`. Make sure to instantiate it with either `Ok`
//      * or `Error` before using it.
//      *
//      * @tparam ErrorType The type of the error message that can be held.
//      */
//     template <typename ErrorType> 
//     class Result<void, ErrorType>
//     {
//         using Flags = U8;

//         enum FlagBits : Flags
//         {
//             RESULT_FLAG_OK      = 1 << 0,
//             RESULT_FLAG_ENGAGED = 1 << 1
//         };

//     public:
//         /**
//          * @brief construct a `Result` object with no error.
//          *
//          */
//         static Result Ok()
//         {
//             Result result{};
//             {
//                 result.m_Flags = RESULT_FLAG_ENGAGED | RESULT_FLAG_OK;
//             }

//             return result;
//         }

//         /**
//          * @brief construct a `Result` object with an error message of type `ErrorType`.
//          *
//          * @param args The arguments to pass to the constructor of `ErrorType`.
//          */
//         template <typename... ErrorArgs> 
//         static Result Error(ErrorArgs &&...args)
//         {
//             Result result{};
//             {
//                 result.m_Flags = RESULT_FLAG_ENGAGED;
//                 result.m_Error.construct(std::forward<ErrorArgs>(args)...);
//             }

//             return result;
//         }

//         Result(const Result& other)
//             requires(std::copy_constructible<ErrorType>)
//             : m_Flags(other.m_Flags)
//         {
//             if (isError())
//             {
//                 m_Error.construct(*other.m_Error.get());
//             }
//         }

//         Result(Result&& other)
//             requires(std::move_constructible<ErrorType>)
//             : m_Flags(other.m_Flags)
//         {
//             if (isError())
//             {
//                 m_Error.construct(std::move(*other.m_Error.get()));
//             }
//         }

//         Result& operator=(const Result& other)
//             requires(std::is_copy_assignable_v<ErrorType>)
//         {
//             if (this == &other)
//             {
//                 return *this;
//             }

//             destroy();
//             m_Flags = other.m_Flags;

//             if (isError())
//             {
//                 m_Error.construct(*other.m_Error.get());
//             }

//             return *this;
//         }

//         Result& operator=(Result&& other)
//             requires(std::is_move_assignable_v<ErrorType>)
//         {
//             if (this == &other)
//             {
//                 return *this;
//             }

//             destroy();
//             m_Flags = other.m_Flags;

//             if (isError())
//             {
//                 m_Error.construct(std::move(*other.m_Error.get()));
//             }

//             return *this;
//         }

//         ~Result()
//         {
//             destroy();
//         }

//         /**
//          * @brief Check if the result is valid.
//          *
//          */
//         bool isOk() const
//         {
//             return checkFlag(RESULT_FLAG_ENGAGED) && checkFlag(RESULT_FLAG_OK);
//         }

//         bool isError() const
//         {
//             return checkFlag(RESULT_FLAG_ENGAGED) && !checkFlag(RESULT_FLAG_OK);
//         }

//         /**
//          * @brief get the error message of the result.
//          *
//          * If the result is valid, this will cause undefined behavior.
//          *
//          */
//         const ErrorType& error() const
//         {
//             VY_ASSERT(isError(), "[RESULT] Result is not an error");

//             return *m_Error.get();
//         }

//         operator bool() const
//         {
//             return isOk();
//         }

//     private:
//         Result() = default;

//         bool checkFlag(const FlagBits flag) const
//         {
//             return m_Flags & flag;
//         }

//         void destroy()
//         {
//             if (isError())
//             {
//                 m_Error.destruct();
//             }
//         }

//         Storage<ErrorType> m_Error;
//         Flags              m_Flags = 0;
//     };
// }