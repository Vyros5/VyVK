#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
    struct PostProcessingComponent 
    {
        // Bloom
        bool  BloomEnabled   { true };
        float BloomThreshold { 1.0f }; // HDR threshold
        float BloomIntensity { 0.7f };
        int   BloomIterations{ 5    }; // blur passes

        // Tonemapping / exposure / gamma
        float Exposure { 1.0f };
        float Gamma    { 2.2f };

        // Color adjustments
        float Vibrance  { 0.0f };
        float Saturation{ 0.0f };
        float Contrast  { 0.0f };
    };
}