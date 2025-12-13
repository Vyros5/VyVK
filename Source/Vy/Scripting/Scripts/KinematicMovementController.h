#pragma once

#include <Vy/Core/Input/Input.h>
#include <Vy/Scripting/EntityScript.h>

namespace Vy
{
	// class KinematcMovementController : public VyEntityScript
	// {
	// public:

	// 	/**
	// 	 * @brief Default keybindings for movement and view control.
	// 	 */
	// 	struct KeyMappings
	// 	{
	// 		VyInput::Key MoveLeft            = VyInput::A;
	// 		VyInput::Key MoveRight           = VyInput::D;
	// 		VyInput::Key MoveForward         = VyInput::W;
	// 		VyInput::Key MoveBackward        = VyInput::S;
	// 		VyInput::Key MoveUp              = VyInput::E;
	// 		VyInput::Key MoveDown            = VyInput::Q;

	// 		VyInput::Key Accelerate          = VyInput::LeftShift;

	// 		VyInput::MouseButton MouseRotate = VyInput::MouseRight;
	// 		VyInput::MouseButton MousePan    = VyInput::MouseMiddle;

	// 		// Disabled by default
	// 		VyInput::Key LookLeft            = VyInput::Left;
	// 		VyInput::Key LookRight           = VyInput::Right;
	// 		VyInput::Key LookUp              = VyInput::Up;
	// 		VyInput::Key LookDown            = VyInput::Down;
	// 	};

	// 	void begin() override;

	// 	void update(float deltaTime) override;

	// 	/// @brief Sets new overall move speed
	// 	void setMoveSpeed(float speed) 
	// 	{ 
	// 		m_MoveSpeed = speed; 
	// 	}

	// 	/// @brief Sets new overall look speed
	// 	void setLookSpeed(float speed) 
	// 	{ 
	// 		m_LookSpeed = speed; 
	// 	}
		
	// 	/// @brief Sets speed for looking with the mouse
	// 	void setMouseSensitivity(float sensitivity) 
	// 	{ 
	// 		m_MouseSensitivity = sensitivity; 
	// 	}

	// 	void setAcceleration(float accel)
	// 	{
	// 		m_Acceleration *= accel;
	// 		m_IsAccelDirty = true;
	// 	}

	// 	void resetAcceleration()
	// 	{
	// 		m_Acceleration = Vec3(1.0f);
	// 		m_IsAccelDirty = false;
	// 	}

	// private:
	// 	void applyRotation(float deltaTime);
	// 	void applyMovement(float deltaTime);

	// 	void apply(float deltaTime);

	// 	void toggleMouseRotate(bool enabled);
	// 	void toogleMousePan   (bool enabled);

	// 	KeyMappings m_Keys{};
	// 	float       m_MoveSpeed{ 6.0f };
	// 	float       m_LookSpeed{ 4.0f };
	// 	float       m_MouseSensitivity{ 0.2f };
	// 	bool        m_MouseRotateEnabled{ false };
	// 	bool        m_MousePanEnabled{ false };

	// 	Vec3        m_Acceleration{ 1.0f };
	// 	bool        m_IsAccelDirty{ false };
	// 	bool        m_FirstMouse{ true };
	// 	Vec2        m_PreviousCursorPos{ 0.0f };

    //     float m_Yaw  { 0.0f };
    //     float m_Pitch{ 0.0f };
	// };
}