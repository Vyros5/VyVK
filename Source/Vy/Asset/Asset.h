#pragma once

#include <VyLib/VyLib.h>
#include <VyLib/Common/VyUUID.h>
#include <VyLib/Serialization/Serializer.h>

namespace Vy
{
	struct VyAssetHandle
	{
		VyUUID ID;

		static VyAssetHandle Invalid() 
        { 
            return { VyUUID::Default() }; 
        }

		bool isValid() const 
        { 
            return ID.isValid(); 
        }

		bool operator==(const VyAssetHandle& other) const 
        { 
            return ID == other.ID; 
        }

		Json::Value serialize() const 
        {
			Json::Value value;
			value["ID"] = ID.toString();

			return value;
		}

		static Optional<VyAssetHandle> deserialize(const Json::Value& value) 
        {
			if (value["ID"].isNull())
            {
                return {};
            }

            return VyAssetHandle{ VyUUID::fromString(value["ID"].asString()) };
		}
	};


	class VyAsset
	{
	public:
        
        VyAsset() = default;
		
        explicit VyAsset(VyAssetHandle assetHandle) : 
            m_Handle{ std::move(assetHandle) }
        {
        }
		
		virtual ~VyAsset() = default;

		const VyAssetHandle& assetHandle() const 
        { 
            return m_Handle; 
        }

		virtual void reload() = 0;

		// Returns false if any asset dependency is invalid.
		virtual bool checkForDirtyInDependencies() = 0; 

	protected:
		VyAssetHandle m_Handle;
	};
}

namespace std 
{
	template<>
	struct hash<Vy::VyAssetHandle> 
    {
		size_t operator()(const Vy::VyAssetHandle& handle) const noexcept 
        {
			return handle.ID.hash();
		}
	};
}