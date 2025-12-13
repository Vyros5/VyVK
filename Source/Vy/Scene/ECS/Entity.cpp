#include <Vy/Scene/ECS/Entity.h>
#include <Vy/Scene/Scene.h>

#include <Vy/Scene/ECS/Components.h>

#include <Vy/Scripting/EntityScript.h>

namespace Vy
{
	VyEntity::VyEntity(EntityHandle handle, VyScene* scene) : 
        m_Handle{ handle }, 
        m_Scene { scene }
	{
		VY_ASSERT(m_Scene, "VyEntity must have a scene");
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


	void VyEntity::addScript(VyEntityScript* script)
	{
		script->m_Entity = *this;
		
		m_Scene->addScript(script);
	}
}