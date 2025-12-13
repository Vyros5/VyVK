#pragma once

#include <VyLib/Common/Numeric.h>
#include <VyLib/STL/String.h>

#include <VyLib/Extern/uuid.h>

namespace Vy
{
    /**
     * @brief A 64-bit universally-unique identifier implementation.
     * 
     * Primarily used by the engine's ECS systems (e.g. in Scene and ScriptEngine).
     */
	class VyID
	{
	public:

        using value_type = U64;

		/**
		 * @brief Default constructor, Generates pseudorandom id value.
		 */
        VyID();

		/**
		 * @brief Construct with assigned id value.
		 * @param uuid The value to be used as the underlying id value.
		 */
        VyID(U64 id);

		/**
		 * @brief Copy constructor.
		 */
        VyID(const VyID& other);

		/**
		 * @brief Cast to underlying id value.
		 */
		operator       U64()       { return m_UUID; }
		operator const U64() const { return m_UUID; }

        bool operator==(const VyID& other) const 
        { 
            return m_UUID == other.m_UUID; 
        }
        
        bool operator!=(const VyID& other) const 
        { 
            return m_UUID != other.m_UUID; 
        }

        bool operator==(U64 val) const 
        { 
            return m_UUID == val; 
        }
        
        bool operator!=(U64 val) const 
        { 
            return m_UUID != val; 
        }
        
        value_type getID() const 
        { 
            return m_UUID; 
        }

        String toString() const
        {
            return std::to_string((Vy::U64)m_UUID);
        }
        
        bool isValid() const 
        { 
            return m_UUID != 0; 
        }
        
    private:
        value_type m_UUID = 0;
    };

    static const VyID::value_type kNullUUID = 0;



	class VyUUID
	{
	public:
		VyUUID();

		explicit VyUUID(const uuids::uuid& uuid);
		
        VyUUID(VyUUID&& other) noexcept;
        VyUUID& operator=(VyUUID&& other) noexcept;
		
        VyUUID(const VyUUID&)            = default;
        VyUUID& operator=(const VyUUID&) = default;

		static VyUUID Default() { return VyUUID(uuids::uuid()); };
        
		static VyUUID fromString(const String& strUuid);
		
        String toString() const 
        { 
            return uuids::to_string(m_UUID); 
        }

		bool isValid() const 
        { 
            return !m_UUID.is_nil(); 
        }

		std::size_t hash() const 
        { 
            return std::hash<uuids::uuid>{}(m_UUID); 
        }

		inline bool operator==(const VyUUID& rhs) const 
        { 
            return this->m_UUID == rhs.m_UUID; 
        }

	private:
		uuids::uuid m_UUID;
		friend std::hash<VyUUID>;
	};
}


namespace std 
{
	template <>
	struct hash<Vy::VyID>
	{
		std::size_t operator()(const Vy::VyID& id) const
		{
			return hash<Vy::U64>()(id.getID());
		}
	};


	template<>
	struct hash<Vy::VyUUID>
	{
		std::size_t operator()(const Vy::VyUUID& uuid) const
		{
			static auto h = std::hash<uuids::uuid>{};
			return h(uuid.m_UUID);
		}
	};
}