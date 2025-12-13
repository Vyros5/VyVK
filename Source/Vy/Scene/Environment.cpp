#include <Vy/Scene/Environment.h>

namespace Vy
{
    void VyEnvironment::setSkybox(const TArray<String, 6>& skyboxTextures)
    {
        m_Skybox = VySkybox::create(skyboxTextures);
    }
}