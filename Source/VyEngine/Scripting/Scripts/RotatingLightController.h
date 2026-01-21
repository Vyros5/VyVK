#pragma once

#include <VyEngine/Scripting/EntityScript.h>

namespace Vy
{
    class RotatingLightController : public VyEntityScript 
    {
    public:
        void begin() override;

        void update(float deltaTime) override;

    private:
        float m_BaseAngle = 0.0f;
        float m_Angle     = 0.0f;
    };
}