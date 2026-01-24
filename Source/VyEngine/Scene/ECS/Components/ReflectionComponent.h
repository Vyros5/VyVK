#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
    struct UVReflectionComponent
    {
        float Reflection{ 0.0f };

		UVReflectionComponent()                    = default;
		UVReflectionComponent(const UVReflectionComponent&) = default;
		
		UVReflectionComponent(float value) : Reflection(value) {}
    };
}