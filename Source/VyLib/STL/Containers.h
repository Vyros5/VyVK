#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/Core/Defines.h>

#include <array>
#include <vector>

#include <map>
#include <unordered_map>

#include <set>
#include <unordered_set>

#include <span>

#include <queue>
#include <deque>

#include <stack>

#include <bitset>

#define InvalidIndex -1

namespace Vy
{
    // --------------------------------------------------------------------------------------------

    /**
     * @brief Doubly-linked list.
     * 
     * @tparam T 
     * @tparam SizeType 
     * 
     * https://en.cppreference.com/w/cpp/container/array.html
     */
    template <typename T, size_t SizeType>
    using TArray = std::array<T, SizeType>;

    template <typename T> using TArray2   = TArray<T,   2>;
    template <typename T> using TArray4   = TArray<T,   4>;
    template <typename T> using TArray8   = TArray<T,   8>;
    template <typename T> using TArray16  = TArray<T,  16>;
    template <typename T> using TArray32  = TArray<T,  32>;
    template <typename T> using TArray64  = TArray<T,  64>;
    template <typename T> using TArray128 = TArray<T, 128>;

    // --------------------------------------------------------------------------------------------

    /**
     * @brief A sequence container that encapsulates dynamic size arrays / resizable contiguous array.
     * 
     * @tparam T 
     * @tparam AllocatorType 
     */
    template <typename T, typename AllocatorType = std::allocator<T>>
    using TVector = std::vector<T, AllocatorType>;

    // --------------------------------------------------------------------------------------------

    /// @brief Collection of key-value pairs, sorted by keys, keys are unique.
    ///        https://en.cppreference.com/w/cpp/container/map.html
    template <
        typename KeyType, 
        typename ValueType, 
        typename CompareType   = std::less<KeyType>, 
        typename AllocatorType = std::allocator<std::pair<const KeyType, ValueType>>
    > 
    using TMap = std::map<KeyType, ValueType, CompareType, AllocatorType>;

    // --------------------------------------------------------------------------------------------

    /// @brief Collection of key-value pairs, sorted by keys, keys are unique.
    ///        https://en.cppreference.com/w/cpp/container/unordered_map.html
    template <
        typename KeyType, 
        typename ValueType,
        typename HashType      = std::hash<KeyType>,
        typename KeyEqualType  = std::equal_to<KeyType>,
        typename AllocatorType = std::allocator<std::pair<const KeyType, ValueType>>
    > 
    using THashMap = std::unordered_map<KeyType, ValueType, HashType, KeyEqualType, AllocatorType>;

    // --------------------------------------------------------------------------------------------

    /// @brief 
    ///        https://en.cppreference.com/w/cpp/container/set.html
    template <
        typename KeyType, 
        typename CompareType   = std::less<KeyType>, 
        typename AllocatorType = std::allocator<KeyType>
    > 
    using TSet = std::set<KeyType, CompareType, AllocatorType>;

    // --------------------------------------------------------------------------------------------

    /// @brief 
    ///        https://en.cppreference.com/w/cpp/container/unordered_set.html
    template <
        typename KeyType, 
        typename HashType      = std::hash<KeyType>,
        typename KeyEqualType  = std::equal_to<KeyType>,
        typename AllocatorType = std::allocator<KeyType>
    > 
    using THashSet = std::unordered_set<KeyType, HashType, KeyEqualType, AllocatorType>;

    // --------------------------------------------------------------------------------------------

    /// @brief A non-owning view over a contiguous sequence of objects.
    ///        https://en.cppreference.com/w/cpp/container/span.html
    template <typename T, size_t ExtentType = std::dynamic_extent>
    using TSpan = std::span<T, ExtentType>;

    // --------------------------------------------------------------------------------------------

    template <typename T> 
    using TDeque = std::deque<T>;
    
    // --------------------------------------------------------------------------------------------

    /// @brief Adapts a container to provide queue (FIFO data structure).
    ///        
    template <typename T, typename ContainerType = TDeque<T>>
    using TQueue = std::queue<T, ContainerType>;

    // --------------------------------------------------------------------------------------------

    template <typename T, typename ContainerType = TVector<T>> 
    using TStack = std::stack<T, ContainerType>;

    // --------------------------------------------------------------------------------------------

    template <size_t N> 
    using TBitSet = std::bitset<N>;

    // --------------------------------------------------------------------------------------------

    using TBitVector = TVector<bool>;

    // --------------------------------------------------------------------------------------------

    using TBlob = TVector<U8>;

    //-------------------------------------------------------------------------
    // Simple utility functions to improve syntactic usage of container types
    //-------------------------------------------------------------------------

    template<typename T>
    concept ContiguousContainer = requires(const T & t)
    {
        { t.data() } -> std::convertible_to<const void*>;
        { t.size() } -> std::convertible_to<size_t>;
    };

    template<typename T>
    concept TriviallyComparableVector =
        requires(const T& v) 
        {
            { v.data() } -> std::convertible_to<const void*>;
            { v.size() } -> std::convertible_to<size_t>;
            typename T::value_type;
        } &&
        std::is_trivially_copyable_v<typename T::value_type>;

    //---------------------------------------------------------------------------------------------
    
    namespace Utils
    {
        // Find an element in a vector
        template<typename T>
        inline typename TVector<T>::const_iterator 
        vectorFind(
            TVector<T> const& vector, 
            T          const& value)
        {
            return std::find(vector.begin(), vector.end(), value);
        }

        // Find an element in a vector
        // Usage: vectorFind(vector, value, [] (T const& typeRef, ValueType const& valueRef) { ... });
        template<typename T, typename ValueType, typename PredicateType>
        inline typename TVector<T>::const_iterator
        vectorFind(
            TVector<T> const& vector, 
            ValueType  const& value, 
            PredicateType     predicate)
        {
            return std::find(vector.begin(), vector.end(), value, std::forward<PredicateType>(predicate));
        }

        // Find an element in a vector
        // Usage: vectorFind(vector, value, [] (T const& typeRef, ValueType const& valueRef) { ... });
        // Require non-const versions since we might want to modify the result
        template<typename T, typename ValueType, typename PredicateType>
        inline typename TVector<T>::iterator 
        vectorFind(
            TVector<T>&      vector, 
            ValueType const& value, 
            PredicateType    predicate)
        {
            return std::find(vector.begin(), vector.end(), value, std::forward<PredicateType>(predicate));
        }

        // Find an element in a vector
        // Require non-const versions since we might want to modify the result
        template<typename T>
        inline typename TVector<T>::iterator 
        vectorFind(
            TVector<T>& vector, 
            T const&    value)
        {
            return std::find(vector.begin(), vector.end(), value);
        }

        //---------------------------------------------------------------------------------------------

        // Usage: vectorContains(vector, value, [] (T const& typeRef, ValueType const& valueRef) { ... });
        template<typename T, typename ValueType, typename PredicateType>
        inline bool 
        vectorContains(
            TVector<T> const& vector, 
            ValueType  const& value, 
            PredicateType     predicate)
        {
            return std::find(vector.begin(), vector.end(), value, std::forward<PredicateType>(predicate)) != vector.end();
        }

        // Usage: vectorContains(vector, [] (T const& typeRef) { ... });
        template<typename T, typename ValueType, typename PredicateType>
        inline bool 
        vectorContains(
            TVector<T> const& vector, 
            PredicateType     predicate)
        {
            return std::find_if(vector.begin(), vector.end(), std::forward<PredicateType>(predicate)) != vector.end();
        }


        template<typename T, typename ValueType>
        inline bool 
        vectorContains(
            TVector<T> const& vector, 
            ValueType  const& value)
        {
            return std::find(vector.begin(), vector.end(), value) != vector.end();
        }

        //---------------------------------------------------------------------------------------------
        
        template<typename T>
        inline I32 
        vectorFindIndex(
            TVector<T> const& vector, 
            T          const& value)
        {
            auto iter = std::find(vector.begin(), vector.end(), value);

            if (iter == vector.end())
            {
                return InvalidIndex;
            }
            else
            {
                return (I32)(iter - vector.begin());
            }
        }

        // Usage: vectorFindIndex(vector, value, [] (T const& typeRef, ValueType const& valueRef) { ... });
        template<typename T, typename ValueType, typename PredicateType>
        inline I32 
        vectorFindIndex(
            TVector<T> const& vector, 
            ValueType  const& value, 
            PredicateType     predicate)
        {
            auto iter = std::find(vector.begin(), vector.end(), value, predicate);

            if (iter == vector.end())
            {
                return InvalidIndex;
            }
            else
            {
                return (I32)(iter - vector.begin());
            }
        }

        // Usage: vectorContains(vector, [] (T const& typeRef) { ... });
        template<typename T, typename ValueType, typename PredicateType>
        inline I32 
        vectorFindIndex(
            TVector<T> const& vector, 
            PredicateType     predicate)
        {
            auto iter = std::find_if(vector.begin(), vector.end(), predicate);

            if (iter == vector.end())
            {
                return InvalidIndex;
            }
            else
            {
                return (I32)(iter - vector.begin());
            }
        }

        //---------------------------------------------------------------------------------------------
        

        template<typename T, typename ValueType>
        inline void 
        TVectorRemove(
            T&               vector, 
            const ValueType& value)
        {
            auto it = std::find(vector.begin(), vector.end(), value);

            if (it != vector.end())
            {
                vector.erase(it);
            }
        }


        template<typename T>
        inline void 
        vectorRemoveAtIndex(
            TVector<T>& vector, 
            U32         index)
        {
            vector.erase(vector.begin() + index);
        }

        //---------------------------------------------------------------------------------------------
        
        /** 
         * Checks if two vectors are equal using std::memcmp for trivially copyable types,
         * otherwise falls back to std::equal.
         */
        template <typename T>
        VY_NODISCARD constexpr bool 
        vectorsAreEqual(const T& A, const T& B)
        {
            if (A.size() != B.size())
            {
                return false;
            }
            
            return std::equal(A.begin(), A.end(), B.begin());
        }

        /** 
         * Checks if two vectors with trivially copyable elements are equal using std::memcmp.
         * This is a more restrictive version that enforces the trivially copyable constraint at compile time.
         */
        template <typename T>
            requires(std::is_trivially_copyable_v<typename T::value_type>)
        VY_NODISCARD bool 
        vectorsAreTriviallyEqual(const T& A, const T& B)
        {
            if (A.size() != B.size())
            {
                return false;
            }
        
            using ValueType = T::value_type;

            return std::memcmp(A.data(), B.data(), A.size() * sizeof(ValueType)) == 0;
        }

        template<typename T, typename ValueType, size_t SizeType>
        VY_NODISCARD bool 
        vectorContains(
            TArray<T, SizeType> const& vector, 
            ValueType           const& value)
        {
            return std::find(vector.begin(), vector.end(), value) != vector.end();
        }

        template<typename T, size_t SizeType, typename ValueType, typename PredicateType>
        VY_NODISCARD bool 
        vectorContains(
            TArray<T, SizeType> const& vector, 
            ValueType           const& value, 
            PredicateType              predicate)
        {
            return std::find(vector.begin(), vector.end(), value, std::forward<PredicateType>(predicate)) != vector.end();
        }

        // Find an element in a vector
        template<typename T, typename ValueType, size_t SizeType>
        VY_NODISCARD typename TArray<T, SizeType>::const_iterator 
        vectorFind(
            TArray<T, SizeType> const& vector, 
            ValueType           const& value)
        {
            return std::find(vector.begin(), vector.end(), value);
        }

        // Find an element in a vector
        template<typename T, typename ValueType, size_t SizeType, typename PredicateType>
        VY_NODISCARD typename TArray<T, SizeType>::const_iterator 
        vectorFind(
            TArray<T, SizeType> const& vector, 
            ValueType           const& value, 
            PredicateType              predicate)
        {
            return std::find(vector.begin(), vector.end(), value, std::forward<PredicateType>(predicate));
        }

        // Find an element in a vector
        // Require non-const versions since we might want to modify the result
        template<typename T, typename ValueType, size_t SizeType>
        VY_NODISCARD typename TArray<T, SizeType>::iterator
        vectorFind(
            TArray<T, SizeType>& vector, 
            ValueType const&     value)
        {
            return std::find(vector.begin(), vector.end(), value);
        }

        // Find an element in a vector
        // Require non-const versions since we might want to modify the result
        template<typename T, typename ValueType, size_t SizeType, typename PredicateType>
        VY_NODISCARD typename TArray<T, SizeType>::iterator 
        vectorFind(
            TArray<T, SizeType>& vector, 
            ValueType const&     value, 
            PredicateType        predicate)
        {
            return std::find(vector.begin(), vector.end(), value, std::forward<PredicateType>(predicate));
        }

        template<typename T, typename ValueType, size_t SizeType>
        VY_NODISCARD I32 
        vectorFindIndex(
            TArray<T, SizeType> const& vector, 
            ValueType           const& value)
        {
            auto iter = std::find(vector.begin(), vector.end(), value);

            if (iter == vector.end())
            {
                return InvalidIndex;
            }
            else
            {
                return (I32) (iter - vector.begin());
            }
        }

        template<typename T, typename ValueType, size_t SizeType, typename PredicateType>
        VY_NODISCARD I32 
        vectorFindIndex(
            TArray<T, SizeType> const& vector, 
            ValueType           const& value, 
            PredicateType              predicate)
        {
            auto iter = std::find(vector.begin(), vector.end(), value, predicate);
            
            if (iter == vector.end())
            {
                return InvalidIndex;
            }
            else
            {
                return (I32) (iter - vector.begin());
            }
        }

        //-------------------------------------------------------------------------

        template<typename T>
        inline void vectorEmplaceBackUnique(TVector<T>& vector, T&& item)
        {
            if (!vectorContains(vector, item))
            {
                vector.emplace_back(item);
            }
        }

        template<typename T>
        inline void vectorEmplaceBackUnique(TVector<T>& vector, T const& item)
        {
            if (!vectorContains(vector, item))
            {
                vector.emplace_back(item);
            }
        }

        template<typename T, size_t SizeType>
        inline void vectorEmplaceBackUnique(TArray<T,SizeType>& vector, T&& item)
        {
            if (!vectorContains(vector, item))
            {
                vector.emplace_back(item);
            }
        }

        template<typename T, size_t SizeType>
        inline void vectorEmplaceBackUnique(TArray<T, SizeType>& vector, T const& item)
        {
            if (!vectorContains(vector, item))
            {
                vector.emplace_back(item);
            }
        }
    } // namespace Utils
}