#include <VyEngine/Scene/ECS/Entity.h>
#include <VyEngine/Scene/Scene.h>

#include <VyEngine/Scene/ECS/Components.h>
#include <VyEngine/Scripting/EntityScript.h>

namespace Vy
{
	VyEntity::VyEntity(
		EntityHandle handle, 
		VyScene*     pScene
	) : 
        m_Handle{ handle }, 
        m_Scene { pScene }
	{
		VY_ASSERT(m_Scene, "VyEntity must have a scene!");
	}

 
    VyEntity::~VyEntity()
    {
    }


	bool VyEntity::operator==(const VyEntity& other) const
	{
		return m_Handle == other.m_Handle && 
		       m_Scene  == other.m_Scene;
	}


	bool VyEntity::operator!=(const VyEntity& other) const
	{
		return !(*this == other);
	}


    entt::registry& VyEntity::registry() const
    {
        return m_Scene->m_Registry;
    }


	VyUUID VyEntity::entityUUID() 
	{
		return this->get<IDComponent>().ID;
	}
	

	TString VyEntity::entityName()
	{
		return this->get<TagComponent>().Tag;
	}


	void VyEntity::addScript(VyEntityScript* pScript)
	{
		pScript->m_Entity = *this;
		
		m_Scene->addScript( pScript );
	}
}