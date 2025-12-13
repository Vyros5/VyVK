#include <Vy/Scripting/Scripts/RotatingLightController.h>

#include <Vy/Scene/ECS/Components.h>

namespace Vy
{
    void RotatingLightController::begin()
    {
        auto& transform = get<TransformComponent>();

        if (transform.Translation.z == 0.0f) 
        {
            m_BaseAngle = 0.0f;
        }
        if (transform.Translation.z == 0.5f) 
        {
            m_BaseAngle = 2.0f * glm::pi<float>()/ 3.0f;
        }
        if (transform.Translation.z == -0.5f) 
        {
            m_BaseAngle = -2.0f * glm::pi<float>()/ 3.0f;
        }
    }

    
    void RotatingLightController::update(float deltaTime)
    {
        auto& transform = get<TransformComponent>();

        transform.Translation.x = 0.5f * glm::cos(m_Angle + m_BaseAngle);
        transform.Translation.z = 0.5f * glm::sin(m_Angle + m_BaseAngle);

        m_Angle = glm::mod(m_Angle + deltaTime, glm::two_pi<float>());
    }
}