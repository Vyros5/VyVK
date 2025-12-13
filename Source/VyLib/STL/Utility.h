#pragma once

#include <utility>
#include <optional>
#include <variant>
#include <functional>

namespace Vy
{
    // --------------------------------------------------------------------------------------------
    
    /**
     * @brief A fixed-size collection of heterogeneous values.
     *        https://en.cppreference.com/w/cpp/utility/tuple.html
     */
    template <typename... Ts>
    using Tuple = std::tuple<Ts...>;
    
    /**
     * @brief Creates a `Vy::Tuple` object, deducing the target type from the types of arguments. 
     *        https://en.cppreference.com/w/cpp/utility/tuple/make_tuple.html
     */
    template<typename... Ts>
    Tuple<Ts...> MakeTuple(Ts&&... args)
    {
        return Tuple<Ts...>(std::forward<Ts>(args)...);
    }

    // --------------------------------------------------------------------------------------------
    
    /// @brief Stores two heterogeneous objects as a single unit.
    ///        https://en.cppreference.com/w/cpp/utility/pair.html
    template <typename T, typename U>
    using Pair = std::pair<T, U>;

    /// @brief Creates a `Vy::Pair` object, deducing the target type from the types of arguments.
    ///        https://en.cppreference.com/w/cpp/utility/pair/make_pair.html
    template <typename T, typename U>
    Pair<T, U> MakePair(const T tVal, const U uVal)
    {
        return Pair<T&, U&>(tVal, uVal);
    }

    // --------------------------------------------------------------------------------------------

    /// @brief Manages an optional contained value, i.e. a value that may or may not be present. 
    ///        https://en.cppreference.com/w/cpp/utility/optional.html
    template<typename T>
    using Optional = std::optional<T>;

    // --------------------------------------------------------------------------------------------

    /// @brief Represents a type-safe union.
    ///        https://en.cppreference.com/w/cpp/utility/variant.html
    template <typename... Ts>
    using Variant = std::variant<Ts...>;

    // --------------------------------------------------------------------------------------------

    template <typename T>
    using Function = std::function<T>;

    // --------------------------------------------------------------------------------------------

}