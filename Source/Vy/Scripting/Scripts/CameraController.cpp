#include <Vy/Scripting/Scripts/CameraController.h>

#include <Vy/Scene/ECS/Components.h>

namespace Vy
{
	void CameraController::begin()
	{
		m_PreviousCursorPos = VyInput::get().cursorPosition();
	}


	void CameraController::update(float deltaTime)
	{
        // Get the transform associated with this script entity.
		auto& transform = get<TransformComponent>();

        // Toggle mouse rotation.
		toggleMouseRotate(VyInput::get().mouseButtonPressed(m_Keys.MouseRotate));
		
        // ----------------------------------------------------------------------------------------
        // [ Keyboard Rotation ]

        Vec3 rotate{ 0.0f };
        if (VyInput::get().keyPressed(m_Keys.LookRight)) 
        {
            rotate.y += 1.0f;
        }
        if (VyInput::get().keyPressed(m_Keys.LookLeft)) 
        {
            rotate.y -= 1.0f;
        }
        if (VyInput::get().keyPressed(m_Keys.LookUp)) 
        {
            rotate.x += 1.0f;
        }
        if (VyInput::get().keyPressed(m_Keys.LookDown)) 
        {
            rotate.x -= 1.0f;
        }

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) 
        {
            transform.Rotation += m_LookSpeed * deltaTime * glm::normalize(rotate);
        }

        // ----------------------------------------------------------------------------------------
        // [ Mouse Rotation ]

        if (m_MouseRotateEnabled)
        {
            auto currentPos = VyInput::get().cursorPosition();

			if (m_FirstMouse) 
			{
				m_PreviousCursorPos = currentPos;
				m_FirstMouse        = false;

                VY_INFO_TAG("CameraController", "First Move");
			}

			// Compute mouse delta. ( Note: glfw's y-coordinate increases downward. )
			Vec2 offset = currentPos - m_PreviousCursorPos;

            m_PreviousCursorPos = currentPos;

			// Invert the Y offset so that moving the mouse up (decreasing y)
			// increases the pitch (Rotation.x) and vice versa.
			transform.Rotation.x += (-offset.y) * m_MouseSensitivity;
			transform.Rotation.y +=   offset.x  * m_MouseSensitivity;

			// Limit pitch values between about +/- 85-ish degrees.
			transform.Rotation.x = glm::clamp(transform.Rotation.x, -1.5f, 1.5f);
			transform.Rotation.y = glm::mod  (transform.Rotation.y, glm::two_pi<float>());

			// Lock rotation around z-axis.
			transform.Rotation.z = 0.0f;
        }

        // ----------------------------------------------------------------------------------------
        // [ Keyboard Translation ]
        
		float yaw = transform.Rotation.y;
		const Vec3 forwardDir{ sin(yaw), 0.0f, cos(yaw)          };
		const Vec3 rightDir  { forwardDir.z, 0.0f, -forwardDir.x };
		const Vec3 upDir     { 0.0f, -1.0f, 0.0f                 };

		Vec3 moveDir{ 0.0f };
		if (VyInput::get().keyPressed(m_Keys.MoveForward)) 
		{
			moveDir += forwardDir;
		}
		if (VyInput::get().keyPressed(m_Keys.MoveBackward)) 
		{
			moveDir -= forwardDir;
		}
		
		if (VyInput::get().keyPressed(m_Keys.MoveRight))    
		{
			moveDir += rightDir;
		}
		if (VyInput::get().keyPressed(m_Keys.MoveLeft))    
		{
			moveDir -= rightDir;
		}

		if (VyInput::get().keyPressed(m_Keys.MoveUp))     
		{
			moveDir += upDir;
		}
		if (VyInput::get().keyPressed(m_Keys.MoveDown))     
		{
			moveDir -= upDir;
		}
		
		if (VyInput::get().keyPressed(m_Keys.Accelerate))     
		{
			m_MoveSpeed = 6.0f;
		}
		else
		{
			m_MoveSpeed = 3.0f;
		}

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) 
		{
			transform.Translation += m_MoveSpeed * deltaTime * glm::normalize(moveDir);
		}
	}


	void CameraController::toggleMouseRotate(bool enabled)
	{
		// if (m_MousePanEnabled)
		// {
		// 	return;
		// }

		if (!m_MouseRotateEnabled && enabled)
		{
			VyInput::get().setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			m_MouseRotateEnabled = true;
			m_PreviousCursorPos  = VyInput::get().cursorPosition();
		}
		else if (m_MouseRotateEnabled && !enabled)
		{
			VyInput::get().setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			m_MouseRotateEnabled = false;
		}
	}
}