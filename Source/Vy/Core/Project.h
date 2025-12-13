#pragma once

#include <VyLib/VyLib.h>
#include <VyLib/STL/Path.h>

#include <json/value.h>

namespace Vy
{
	struct VyProjectConfig
	{
		String Name  = "Untitled";
		Path   StartScene;
		Path   AssetDirectory;
	};

	class VyProject
	{
	public:
		const VyProjectConfig& projectConfig() const { return m_Config; }
		const Path&            projectPath()   const { return m_ProjectDirectory; }

		static const Path& projectDirectory()
		{
			VY_ASSERT(s_ActiveProject, "Should not be null");

			return s_ActiveProject->m_ProjectDirectory;
		}

		static const Path& assetDirectory()
		{
			VY_ASSERT(s_ActiveProject, "Should not be null");

			return s_ActiveProject->m_AssetDirectory;
		}

		static Shared<VyProject> activeProject() 
		{ 
			return s_ActiveProject; 
		}

		static Shared<VyProject> newProject();

		static Shared<VyProject> loadProject(const Path& path);
		
		static bool saveActiveProject(const Path& path);

		Json::Value serialize() const;

		static Optional<VyProject> deserialize(const Json::Value& root);
		
	private:

		VyProjectConfig m_Config;
		Path            m_ProjectDirectory;
		Path            m_AssetDirectory;
        
		static Shared<VyProject> s_ActiveProject;
	};
}