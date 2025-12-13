#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/Core/Assert.h>
#include <VyLib/STL/Utility.h>
#include <VyLib/STL/Path.h>

#include <json/json.h>

namespace Vy
{
	template<typename T>
	concept Serializable = requires(T t, const Json::Value& root)
	{
		{ t.serialize() } -> std::same_as<Json::Value>;
		{ T::deserialize(root) } -> std::same_as<std::optional<T>>;
	};


	template <Serializable T>
	class Serializer
	{
	public:
		explicit Serializer(T& serializable) : 
            m_Serializable(serializable) 
        {
        }

		void serialize(const Path& path)
		{
			const Json::Value root = m_Serializable.serialize();
			std::ofstream file(path);
			file << root;
			file.close();
		}

		static Optional<T> deserialize(const Path& path)
		{
			std::ifstream file(path);
			if (!file.is_open())
            {
                return std::nullopt;
            }

			Json::Value root;
			file >> root;
			file.close();

			return T::deserialize(root);
		}

	private:

		T& m_Serializable;
	};
}