#include <VyEngine/Core/Input/Input.h>


namespace Vy
{
	VyInput* VyInput::s_Instance = nullptr;


	VyInput::VyInput(const VyWindow& window) : 
		m_Window{ window.handle() }
	{
		VY_ASSERT(s_Instance == nullptr, "Cannot create VyInput: Only one instance of VyInput is allowed");

		s_Instance = this;

		if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

		// Set up key callback
		glfwSetKeyCallback(m_Window, [](GLFWwindow*, int key, int scancode, int action, int mods)
        {
            VyInput::get().glfwKeyCallback(key, scancode, static_cast<KeyEvent>(action), static_cast<Modifier>(mods));
        });
	}


	VyInput::~VyInput()
	{
		s_Instance = nullptr;
	}


	VyInput& VyInput::get()
	{
		VY_ASSERT(s_Instance != nullptr, "VyInput instance is not created");

		return *s_Instance;
	}


	bool VyInput::keyPressed(Key key)
	{
		return glfwGetKey(m_Window, static_cast<int>(key)) == GLFW_PRESS;
	}


	bool VyInput::mouseButtonPressed(MouseButton button)
	{
		return glfwGetMouseButton(m_Window, static_cast<int>(button)) == GLFW_PRESS;
	}


	Vec2 VyInput::cursorPosition()
	{
		double xPos, yPos;
		glfwGetCursorPos(m_Window, &xPos, &yPos);

		return { xPos, yPos };
	}


	void VyInput::setInputMode(int mode, int value)
	{
		glfwSetInputMode(m_Window, mode, value);
	}

	
	void VyInput::glfwKeyCallback(int key, int scancode, KeyEvent action, Modifier mods)
	{
		for (const auto& binding : m_KeyBindings[ key ])
		{
			if (binding.Event == action && binding.Mods == mods)
            {
                binding.Func();
            }
		}
	}
}