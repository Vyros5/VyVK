#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
    // Simple color component used when no material is bound
    struct ColorComponent 
    {
        Vec3 Color{ 1.0f, 1.0f, 1.0f };
    };
}