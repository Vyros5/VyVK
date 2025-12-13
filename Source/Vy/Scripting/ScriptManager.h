#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
    class VyEntityScript;
    

	class VyScriptManager
	{
	public:
		VyScriptManager() = default;
		
        VyScriptManager(const VyScriptManager&) = delete;
		VyScriptManager(VyScriptManager&&)      = delete;

		~VyScriptManager();

		VyScriptManager& operator=(const VyScriptManager&) = delete;
		VyScriptManager& operator=(VyScriptManager&&)      = delete;

        /**
         * @brief Adds a script to call its virtual functions.
         * 
         * @param script The script to add.
         */
		void addScript(VyEntityScript* script);

        /**
         * @brief Calls the update function of each script.
         * 
         * @param deltaTime Time elapsed since the last update.
         */
		void update(float deltaTime);

	private:

        /**
         * @brief Calls the begin function of each script once.
         */
		void handleNewScripts();
		
		TVector<VyEntityScript*> m_Scripts;
		TVector<VyEntityScript*> m_NewScripts;
	};
}