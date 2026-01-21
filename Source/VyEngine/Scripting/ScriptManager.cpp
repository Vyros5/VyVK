#include <VyEngine/Scripting/ScriptManager.h>

#include <VyEngine/Scripting/EntityScript.h>

namespace Vy
{
	VyScriptManager::~VyScriptManager()
	{
		handleNewScripts(); // Just in case

		for (auto& pScript : m_Scripts)
		{
			pScript->end();
		}
	}
    

	void VyScriptManager::addScript(VyEntityScript* pScript)
	{
		m_NewScripts.emplace_back( pScript );
	}


	void VyScriptManager::update(float deltaTime)
	{
		handleNewScripts();

		for (auto& pScript : m_Scripts)
		{
			pScript->update( deltaTime );
		}
	}
    

	void VyScriptManager::handleNewScripts()
	{
		for (auto& pScript : m_NewScripts)
		{
			pScript->begin();

			m_Scripts.emplace_back( pScript );
		}
		
		m_NewScripts.clear();
	}
}