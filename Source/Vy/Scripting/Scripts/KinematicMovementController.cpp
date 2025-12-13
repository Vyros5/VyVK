#include <Vy/Scripting/Scripts/KinematicMovementController.h>

#include <Vy/Scene/ECS/Components.h>

namespace Vy
{
	// void KinematcMovementController::begin()
	// {
	// 	m_PreviousCursorPos = VyInput::get().cursorPosition();
	// }


	// void KinematcMovementController::update(float deltaTime)
	// {
	// 	apply(deltaTime);

	// 	// applyRotation(deltaTime);
	// 	// applyMovement(deltaTime);
	// }


	// void KinematcMovementController::apply(float deltaTime)
	// {
	// 	auto& transform = get<TransformComponent2>();

	// 	toggleMouseRotate(VyInput::get().mouseButtonPressed(m_Keys.MouseRotate));
		
	// 	Vec3 rotate{ 0.0f };
	// 	if (VyInput::get().keyPressed(m_Keys.LookRight)) 
	// 	{
	// 		rotate.y += 1.0f;
	// 	}
	// 	if (VyInput::get().keyPressed(m_Keys.LookLeft )) 
	// 	{
	// 		rotate.y -= 1.0f;
	// 	}
	// 	if (VyInput::get().keyPressed(m_Keys.LookUp   )) 
	// 	{
	// 		rotate.x += 1.0f;
	// 	}
	// 	if (VyInput::get().keyPressed(m_Keys.LookDown )) 
	// 	{
	// 		rotate.x -= 1.0f;
	// 	}

	// 	if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) 
	// 	{
	// 		transform.Rotation += m_LookSpeed * deltaTime * glm::normalize(rotate);
	// 	}

	// 	if (m_MouseRotateEnabled)
	// 	{
	// 		auto currentPos = VyInput::get().cursorPosition();
			
	// 		if (m_FirstMouse) 
	// 		{
	// 			m_PreviousCursorPos = currentPos;
	// 			m_FirstMouse        = false;
	// 		}
	// 		// Compute mouse delta. (Note: glfw's y-coordinate increases downward.)
	// 		Vec2 offset = currentPos - m_PreviousCursorPos;
			
	// 		m_PreviousCursorPos = currentPos;
			
	// 		// Invert the Y offset so that moving the mouse up (decreasing y)
	// 		// increases the pitch (rotation.x) and vice versa.
	// 		transform.Rotation.x += (-offset.y) * m_MouseSensitivity;
	// 		transform.Rotation.y +=   offset.x  * m_MouseSensitivity;

	// 		transform.Rotation.x = glm::clamp(transform.Rotation.x, -1.5f, 1.5f);
	// 		transform.Rotation.y = glm::mod  (transform.Rotation.y, glm::two_pi<float>());
	// 	}

	// 	// float pitch = transform.Rotation.x;
	// 	float yaw = transform.Rotation.y;
	// 	const Vec3 forwardDir{ sin(yaw), 0.0f, cos(yaw) };
	// 	const Vec3 rightDir  { forwardDir.z, 0.0f, -forwardDir.x };
	// 	const Vec3 upDir     { 0.0f, -1.0f, 0.0f };

	// 	Vec3 moveDir{ 0.0f };
	// 	if (VyInput::get().keyPressed(m_Keys.MoveForward)) 
	// 	{
	// 		moveDir += forwardDir;
	// 	}
	// 	if (VyInput::get().keyPressed(m_Keys.MoveBackward)) 
	// 	{
	// 		moveDir -= forwardDir;
	// 	}
	// 	if (VyInput::get().keyPressed(m_Keys.MoveRight))    
	// 	{
	// 		moveDir += rightDir;
	// 	}
	// 	if (VyInput::get().keyPressed(m_Keys.MoveLeft))    
	// 	{
	// 		moveDir -= rightDir;
	// 	}
	// 	if (VyInput::get().keyPressed(m_Keys.MoveUp))     
	// 	{
	// 		moveDir += upDir;
	// 	}
	// 	if (VyInput::get().keyPressed(m_Keys.MoveDown))     
	// 	{
	// 		moveDir -= upDir;
	// 	}

	// 	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) 
	// 	{
	// 		transform.Position += m_MoveSpeed * deltaTime * glm::normalize(moveDir);
	// 	}
	// }


	// void KinematcMovementController::applyRotation(float deltaTime)
	// {
	// 	auto& transform = get<TransformComponent>();

	// 	///////////////////////////////////////////////////////////////////////////////////////////
	// 	// Key input rotation.

	// 	Vec3 rotate{ 0.0f };
	// 	if (VyInput::get().keyPressed(m_Keys.LookRight)) rotate.y -= m_LookSpeed;
	// 	if (VyInput::get().keyPressed(m_Keys.LookLeft )) rotate.y += m_LookSpeed;
	// 	if (VyInput::get().keyPressed(m_Keys.LookUp   )) rotate.x += m_LookSpeed;
	// 	if (VyInput::get().keyPressed(m_Keys.LookDown )) rotate.x -= m_LookSpeed;

	// 	///////////////////////////////////////////////////////////////////////////////////////////
	// 	// Mouse input rotation.

	// 	toggleMouseRotate(VyInput::get().mouseButtonPressed(m_Keys.MouseRotate));
		
	// 	if (m_MouseRotateEnabled)
	// 	{
	// 		auto cursorPos = VyInput::get().cursorPosition();
			
	// 		// Yaw Delta
	// 		rotate.x += (cursorPos.y - m_PreviousCursorPos.y) * m_MouseSensitivity;

	// 		// Pitch Delta
	// 		rotate.y += (cursorPos.x - m_PreviousCursorPos.x) * m_MouseSensitivity;
			
	// 		m_PreviousCursorPos = cursorPos;
	// 	}

	// 	///////////////////////////////////////////////////////////////////////////////////////////
	// 	// Apply Rotation

    //     // if (glm::abs(rotate.x) >= std::numeric_limits<float>::epsilon() ||
	// 	// glm::abs(rotate.y) >= std::numeric_limits<float>::epsilon())
	// 	if (glm::length(rotate) > std::numeric_limits<float>::epsilon())
	// 	{
	// 		rotate *= deltaTime;
			
	// 		Vec3 rotation = glm::eulerAngles(transform.Rotation);
			
	// 		// Limit pitch values between about +/- 85-ish degrees.
	// 		rotation.x = glm::clamp(rotation.x + rotate.x, -1.5f, 1.5f);
	// 		rotation.y = glm::mod  (rotation.y + rotate.y, glm::two_pi<float>());
	// 		rotation.z = 0.0f; // Lock rotation around z-axis.
			
    //         transform.Rotation = Quat(rotation);
	// 	}
	// }

	
	// void KinematcMovementController::applyMovement(float deltaTime)
	// {
	// 	auto& transform = get<TransformComponent>();
	
	// 	// float yaw = glm::eulerAngles(transform.Rotation).y;
    //     // const Vec3 forwardDir{ glm::sin(yaw), 0.0f, glm::cos(yaw) };
    //     // const Vec3 rightDir  { forwardDir.z,  0.0f, -forwardDir.x  };
    //     // const Vec3 upDir     { 0.0f, -1.0f, 0.0f };

	// 	auto forwardDir = transform.forward();
	// 	auto rightDir   = transform.right();
	// 	auto upDir      = Math::World::UP;

	// 	///////////////////////////////////////////////////////////////////////////////////////////
	// 	// Key input movement.
		
	// 	Vec3 moveDir{ 0.0f };
	// 	if (VyInput::get().keyPressed(m_Keys.MoveForward )) moveDir += forwardDir;
	// 	if (VyInput::get().keyPressed(m_Keys.MoveBackward)) moveDir -= forwardDir;

	// 	if (VyInput::get().keyPressed(m_Keys.MoveRight))    moveDir -= rightDir;
	// 	if (VyInput::get().keyPressed(m_Keys.MoveLeft ))    moveDir += rightDir;
		
	// 	if (VyInput::get().keyPressed(m_Keys.MoveUp  ))     moveDir += upDir;
	// 	if (VyInput::get().keyPressed(m_Keys.MoveDown))     moveDir -= upDir;

	// 	if (VyInput::get().keyPressed(m_Keys.Accelerate))
	// 	{
	// 		m_MoveSpeed = 6.0f;
	// 	}
	// 	else
	// 	{
	// 		m_MoveSpeed = 2.0f;
	// 	}

	// 	// Apply movement.
	// 	if (glm::length(moveDir) > std::numeric_limits<float>::epsilon())
	// 	{
	// 		// transform.Position += m_MoveSpeed * m_Acceleration * glm::normalize(moveDir) * deltaTime;
	// 		transform.Position += m_MoveSpeed * deltaTime * glm::normalize(moveDir);
	// 	}

	// 	///////////////////////////////////////////////////////////////////////////////////////////
	// 	// Mouse pan input movement.

	// 	toogleMousePan(VyInput::get().mouseButtonPressed(m_Keys.MousePan));

	// 	if (m_MousePanEnabled)
	// 	{
	// 		auto cursorPos = VyInput::get().cursorPosition();

	// 		moveDir -= rightDir * (cursorPos.x - m_PreviousCursorPos.x);
	// 		moveDir += upDir    * (cursorPos.y - m_PreviousCursorPos.y);
			
    //         m_PreviousCursorPos = cursorPos;

	// 		// Apply movement.
	// 		if (glm::length(moveDir) > std::numeric_limits<float>::epsilon())
	// 		{
	// 			transform.Position += moveDir * deltaTime;
	// 		}
	// 	}
	// }


	// void KinematcMovementController::toggleMouseRotate(bool enabled)
	// {
	// 	if (m_MousePanEnabled)
	// 	{
	// 		return;
	// 	}

	// 	if (!m_MouseRotateEnabled && enabled)
	// 	{
	// 		VyInput::get().setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// 		m_MouseRotateEnabled = true;
	// 		m_PreviousCursorPos  = VyInput::get().cursorPosition();
	// 	}
	// 	else if (m_MouseRotateEnabled && !enabled)
	// 	{
	// 		VyInput::get().setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// 		m_MouseRotateEnabled = false;
	// 	}
	// }


	// void KinematcMovementController::toogleMousePan(bool enabled)
	// {
	// 	if (m_MouseRotateEnabled)
	// 	{
	// 		return;
	// 	}

	// 	if (!m_MousePanEnabled && enabled)
	// 	{
	// 		VyInput::get().setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// 		m_MousePanEnabled   = true;
	// 		m_PreviousCursorPos = VyInput::get().cursorPosition();
	// 	}
	// 	else if (m_MousePanEnabled && !enabled)
	// 	{
	// 		VyInput::get().setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// 		m_MousePanEnabled = false;
	// 	}
	// }
}