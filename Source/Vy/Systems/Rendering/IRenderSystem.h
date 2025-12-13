#pragma once

#include <Vy/GFX/FrameInfo.h>
#include <Vy/Scene/ECS/Components.h>
#include <Vy/GFX/Backend/Pipeline.h>
#include <Vy/GFX/Backend/Resources/Framebuffer.h>

namespace Vy
{
	class IRenderSystem
	{
	public:
		IRenderSystem()          = default;
		virtual ~IRenderSystem() = default;

		virtual void render(const VyFrameInfo& frameInfo) = 0;

		virtual void update(VyFrameInfo& frameInfo, GlobalUBO& ubo) {}
	};


	template <typename T>
	concept RenderSystemDerived = std::derived_from<T, IRenderSystem>;
}