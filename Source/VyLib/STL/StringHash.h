#pragma once

#include <VyLib/STL/String.h>

namespace Vy
{
	template<typename T = char >
	struct StringHash
	{
		using hash_type      = std::hash<std::basic_string_view<T>>;
		using is_transparent = void;

		std::size_t operator()(const T* str) const;

		std::size_t operator()(std::basic_string_view<T> str) const;
		
        template<typename Allocator> 
        std::size_t operator()(const std::basic_string<T, std::char_traits<T>, Allocator>& str) const;
	};


	template<typename T>
	std::size_t StringHash<T>::operator()(const T* str) const
	{
		return hash_type{}(str);
	}

	template<typename T>
	std::size_t StringHash<T>::operator()(std::basic_string_view<T> str) const
	{
		return hash_type{}(str);
	}
	
	template<typename T>
	template<typename Allocator>
	std::size_t StringHash<T>::operator()(const std::basic_string<T, std::char_traits<T>, Allocator>& str) const
	{
		return hash_type{}(str);
	}
}