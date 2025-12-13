#include <VyLib/Common/VyUUID.h>

#include <random>

namespace Vy
{
    /// @brief Random device for UUID generation.
    static std::random_device s_RandomDevice;

    static std::mt19937 s_RandomEngine(s_RandomDevice());

    /// @brief 64-bit Mersenne Twister random number generator.
    static std::mt19937_64 s_RandomEngine64(s_RandomDevice());

    /// @brief Uniform distribution for generating 64-bit UUID values.
    static std::uniform_int_distribution<U64> s_UniformDistribution64;

    static uuids::uuid_random_generator s_UUIDRandomGenerator(s_RandomEngine);

    VyID::VyID() :
        m_UUID(s_UniformDistribution64(s_RandomEngine64))
    {
    }


    VyID::VyID(U64 id) : 
        m_UUID(id)
    {
    }

    
    VyID::VyID(const VyID& other) : 
        m_UUID(other.m_UUID) 
    {
    }



	VyUUID::VyUUID() 
    {
		m_UUID = s_UUIDRandomGenerator();
	}

	VyUUID::VyUUID(const uuids::uuid& uuid) 
    {
		m_UUID = uuid;
	}

	VyUUID::VyUUID(VyUUID&& other) noexcept
	{
		m_UUID.swap(other.m_UUID);
	}

	VyUUID& VyUUID::operator=(VyUUID&& other) noexcept
	{
		m_UUID.swap(other.m_UUID);
		return *this;
	}

	VyUUID VyUUID::fromString(const String& uuid) 
    {
		auto id = uuids::uuid::from_string(uuid);
		
        if (!id.has_value()) 
        {
			throw std::invalid_argument("VyUUID : " + uuid + " is not a valid id");
		}
		
        return VyUUID(id.value());
	}
}