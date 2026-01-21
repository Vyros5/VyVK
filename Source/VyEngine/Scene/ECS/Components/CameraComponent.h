#pragma once

#include <VyEngine/Scene/Camera.h>
#include <VyEngine/Scene/ECS/EntityHandle.h>

namespace Vy
{
	struct CameraComponent
	{
		VyCamera Camera      {};
		bool     IsMainCamera{ true };

		const Mat4& view()        const { return Camera.view();        }
		const Mat4& inverseView() const { return Camera.inverseView(); }
		const Mat4& projection()  const { return Camera.projection();  }
		const Vec3  position()    const { return Camera.position();    }

		CameraComponent(const CameraComponent&) = default;

		CameraComponent() : 
			IsMainCamera{ true }
		{ 
			VY_INFO_TAG("CameraComponent", "[ Main Camera Initialized ]");
		}
		
		CameraComponent(const VyCamera& camera) : 
			Camera{ camera }
		{
		}
	};
}