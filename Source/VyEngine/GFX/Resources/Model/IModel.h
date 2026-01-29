#pragma once

#include <VyEngine/VK/Device/Device.h>

namespace Vy
{
	class IModel
    {
	public:
		virtual ~IModel() = default;

		virtual void bind(VkCommandBuffer cmdBuffer) {};

		virtual void draw(VkCommandBuffer cmdBuffer) {};
	
        
    };
}