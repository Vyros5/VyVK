#pragma once

#include <Vy/GFX/Resources/StaticMesh.h>
#include <Vy/GFX/Resources/Mesh.h>

namespace Vy
{
	struct ModelComponent 
    {
		Shared<VyStaticMesh> Model;

		ModelComponent() = default;
		ModelComponent(const ModelComponent&) = default;

		ModelComponent(const Shared<VyStaticMesh>& model) : Model(model) {}
	};

	struct ModelComponent2
    {
		Shared<Model> model;

		ModelComponent2() = default;
		ModelComponent2(const ModelComponent2&) = default;

		ModelComponent2(const Shared<Model>& model_) : model(model_) {}
	};
}