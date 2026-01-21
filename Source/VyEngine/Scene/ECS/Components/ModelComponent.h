#pragma once

#include <VyEngine/GFX/Resources/Model/Model.h>
#include <VyEngine/GFX/Resources/Mesh/Model.h>
namespace Vy
{
	struct ModelComponent 
    {
		Shared<VyModel> Model;

		ModelComponent() = delete;
		ModelComponent(const ModelComponent&) = default;

		ModelComponent(const Shared<VyModel>& model) : 
			Model( model ) 
		{
		}

		ModelComponent(const TPath& filepath)
			: Model( VyModel::loadFromFile( filepath ) ) 
		{
		}
	};


	// struct ModelComponent2
    // {
	// 	Shared<VyModel> Model;

	// 	ModelComponent2() = delete;
	// 	ModelComponent2(const ModelComponent2&) = default;

	// 	ModelComponent2(const Shared<VyModel>& model) : 
	// 		Model( model ) 
	// 	{
	// 	}

	// 	ModelComponent2(const TString& filepath)
	// 		: Model( VyModel::createFromFile( filepath ) ) 
	// 	{
	// 	}
	// };


	struct MeshModelComponent 
    {
		Shared<MeshModel> Model;

		MeshModelComponent() = delete;
		MeshModelComponent(const MeshModelComponent&) = default;

		MeshModelComponent(const Shared<MeshModel>& model) : 
			Model( model ) 
		{
		}

		MeshModelComponent(TStringView path)
			: Model( MeshModel::createMeshFromFile( path ) ) 
		{
		}
	};
}