#include <Vy/Core/Project.h>

#include <VyLib/Serialization/Serializer.h>

namespace Vy
{
	Shared<VyProject> 
	VyProject::s_ActiveProject = MakeShared<VyProject>();


	Shared<VyProject> 
	VyProject::newProject()
	{
		s_ActiveProject = MakeShared<VyProject>();
	
        return s_ActiveProject;
	}


	Shared<VyProject> 
	VyProject::loadProject(const Path& path)
	{
		if (const auto project = Serializer<VyProject>::deserialize(path))
		{
			const auto projPtr = MakeShared<VyProject>(*project);

			projPtr->m_ProjectDirectory = canonical(path.parent_path());
			projPtr->m_AssetDirectory   = canonical(projPtr->m_ProjectDirectory / projPtr->m_Config.AssetDirectory);

			s_ActiveProject = projPtr;
			
            return s_ActiveProject;
		}

		return nullptr;
	}


	bool VyProject::saveActiveProject(const Path& path)
	{

		Serializer serializer(*s_ActiveProject);
		serializer.serialize(path);
		s_ActiveProject->m_ProjectDirectory = canonical(path.parent_path());
		s_ActiveProject->m_AssetDirectory = canonical(s_ActiveProject->m_ProjectDirectory / s_ActiveProject->m_Config.AssetDirectory);
		
        return true;
	}


	Json::Value VyProject::serialize() const
	{
		Json::Value root;
		auto& project = root["Project"];
		project["Name"]            = m_Config.Name;
		project["StartScene"]      = m_Config.StartScene.string();
		project["AssetsDirectory"] = m_Config.AssetDirectory.string();

		return root;
	}

    
	Optional<VyProject> VyProject::deserialize(const Json::Value& root)
	{
		if (!root.isMember("Project"))
		{
			return std::nullopt;
		}

		const auto& project = root["Project"];

		if (!project.isMember("Name")            || 
			!project.isMember("StartScene")      || 
			!project.isMember("AssetsDirectory"))
		{
			return std::nullopt;
		}

		VyProjectConfig config;
		{
			config.Name           = project["Name"].asString();
			config.StartScene     = project["StartScene"].asString();
			config.AssetDirectory = project["AssetsDirectory"].asString();
		}

		VyProject out;
		{
			out.m_Config = config;
		}

		return out;
	}
}