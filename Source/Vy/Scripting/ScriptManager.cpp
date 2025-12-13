#include <Vy/Scripting/ScriptManager.h>

#include <Vy/Scripting/EntityScript.h>

namespace Vy
{
	VyScriptManager::~VyScriptManager()
	{
		handleNewScripts(); // just in case

		for (auto& script : m_Scripts)
		{
			script->end();
		}
	}
    

	void VyScriptManager::addScript(VyEntityScript* script)
	{
		m_NewScripts.emplace_back(script);
	}


	void VyScriptManager::update(float deltaTime)
	{
		handleNewScripts();

		for (auto& script : m_Scripts)
		{
			script->update(deltaTime);
		}
	}
    

	void VyScriptManager::handleNewScripts()
	{
		for (auto& script : m_NewScripts)
		{
			script->begin();

			m_Scripts.emplace_back(script);
		}
		
		m_NewScripts.clear();
	}
}