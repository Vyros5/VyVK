#pragma once

#include <Vy/GFX/Resources/StaticMesh.h>

namespace Vy
{
	struct ModelComponent 
    {
		Shared<VyStaticMesh> Model;

		ModelComponent() = default;
		ModelComponent(const ModelComponent&) = default;

		ModelComponent(const Shared<VyStaticMesh>& model) : Model(model) {}
	};
}