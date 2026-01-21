#pragma once

#include <VyEngine/VK/Core/VKCore.h>

namespace Vy 
{
    struct ModelPushConstantData
    {
        Mat4 ModelMatrix { 1.0f };
        Mat4 NormalMatrix{ 1.0f }; // 4x4 because of alignment
    };
}