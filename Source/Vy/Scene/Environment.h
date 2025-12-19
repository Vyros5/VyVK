#pragma once

#include <Vy/GFX/Resources/Cubemap.h>
#include <Vy/Scene/ECS/Components/LightComponent.h>

namespace Vy
{
	class VyEnvironment 
    {
	public:
		/**
		 * @brief Get the ambient light of the environment.
		 * The RGB components represents the ambient light color, the alpha component represents the intensity.
		 *
		 * @return The ambient light.
		 */
		Vec4 getAmbientLight() const { return m_AmbientLight; }

		/**
		 * @brief Set the ambient light of the environment.
		 * The RGB components represents the ambient light color, the alpha component represents the intensity.
		 *
		 * @param ambientLight The ambient light to set.
		 */
		void setAmbientLight(const Vec4& ambientLight) { m_AmbientLight = ambientLight; }

		/**
		 * @brief Get the Skybox of the environment.
		 * 
		 * @return The skybox.
		 */
		Shared<VySkybox>& getSkybox() { return m_Skybox; }

		
		// DirectionalLightComponent getSun() const { return m_Sun; }

		/**
		 * @brief Set the skybox of the environment.
		 * The skybox is used to render the background of the scene.
		 *
		 * @param skyboxTextures An array of texture paths for the skybox faces.
		 */
		void setSkybox(const TArray<String, 6>& skyboxTextures);

	private:
		Vec4             m_AmbientLight = Vec4{ 0.67f, 0.85f, 0.9f, .02f };
		Shared<VySkybox> m_Skybox       = nullptr; 
		// DirectionalLightComponent m_Sun;
	};
}