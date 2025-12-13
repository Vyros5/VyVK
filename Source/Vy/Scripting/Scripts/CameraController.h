#pragma once

#include <Vy/Core/Input/Input.h>
#include <Vy/Scripting/EntityScript.h>

namespace Vy
{
	class CameraController : public VyEntityScript
	{
	public:
		/**
		 * @brief Default keybindings for movement and view control.
		 */
		struct KeyMappings
		{
			VyInput::Key MoveLeft            = VyInput::A;
			VyInput::Key MoveRight           = VyInput::D;
			VyInput::Key MoveForward         = VyInput::W;
			VyInput::Key MoveBackward        = VyInput::S;
			VyInput::Key MoveUp              = VyInput::E;
			VyInput::Key MoveDown            = VyInput::Q;

			VyInput::Key Accelerate          = VyInput::LeftShift;

			VyInput::MouseButton MouseRotate = VyInput::MouseRight;
			VyInput::MouseButton MousePan    = VyInput::MouseMiddle;

			VyInput::Key LookLeft            = VyInput::Left;
			VyInput::Key LookRight           = VyInput::Right;
			VyInput::Key LookUp              = VyInput::Up;
			VyInput::Key LookDown            = VyInput::Down;
		};

		void begin() override;

		void update(float deltaTime) override;


		/// @brief Sets new overall move speed
		void setMoveSpeed(float speed) 
		{ 
			m_MoveSpeed = speed; 
		}

		/// @brief Sets new overall look speed
		void setLookSpeed(float speed) 
		{ 
			m_LookSpeed = speed; 
		}
		
		/// @brief Sets speed for looking with the mouse
		void setMouseSensitivity(float sensitivity) 
		{ 
			m_MouseSensitivity = sensitivity; 
		}

	private:

		void toggleMouseRotate(bool enabled);
		// void toogleMousePan   (bool enabled);

		KeyMappings m_Keys{};

		// Movement
		float       m_MoveSpeed        { 3.0f };
		float       m_AccelMultiplier  { 1.5f };
		float       m_SpeedModifier    { 1.0f };

		float       m_LookSpeed        { 2.0f };
		float       m_MouseSensitivity { 0.0025f };

		Vec2        m_PreviousCursorPos{ 0.0f };

        bool        m_MouseRotateEnabled{ false };
		bool        m_FirstMouse        { true  };

		// bool        m_MousePanEnabled   { false };
	};
}