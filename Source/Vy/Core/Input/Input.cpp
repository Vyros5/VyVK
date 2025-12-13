#include <Vy/Core/Input/Input.h>


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
		return glfwGetKey(m_Window, key) == GLFW_PRESS;
	}


	bool VyInput::mouseButtonPressed(MouseButton button)
	{
		return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
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
		for (const auto& binding : m_KeyBindings[key])
		{
			if (binding.Event == action && binding.Mods == mods)
            {
                binding.Func();
            }
		}
	}
}






// namespace Vy
// {
// 	// --- Static member initialization ---
// 	GLFWwindow* Input::m_Window = nullptr;
// 	THashMap<int, bool> Input::m_KeyState;
// 	THashMap<int, bool> Input::m_PrevKeyState;
// 	TArray<bool, GLFW_MOUSE_BUTTON_LAST + 1> Input::m_MouseButtonState{};
// 	TArray<bool, GLFW_MOUSE_BUTTON_LAST + 1> Input::m_PrevMouseButtonState{};
// 	Vec2 Input::m_MousePos{0.0f, 0.0f};
// 	Vec2 Input::m_PrevMousePos{0.0f, 0.0f};
// 	Vec2 Input::m_MouseDelta{0.0f, 0.0f};
// 	Vec2 Input::m_ScrollDelta{0.0f, 0.0f};
// 	int  Input::m_CursorMode = GLFW_CURSOR_NORMAL;
// 	bool Input::m_FirstFrame = true;

// 	// --- Initialization ---

// 	void Input::init(GLFWwindow* window) 
// 	{
// 		m_Window     = window;
// 		m_FirstFrame = true;

// 		// Get initial mouse position
// 		double x, y;
// 		glfwGetCursorPos(m_Window, &x, &y);
// 		m_MousePos     = Vec2(static_cast<F32>(x), static_cast<F32>(y));
// 		m_PrevMousePos = m_MousePos;

// 		// Set up scroll callback
// 		glfwSetScrollCallback(m_Window, scrollCallback);
// 	}


// 	void Input::cleanup() 
// 	{
// 		m_Window = nullptr;
		
// 		m_KeyState.clear();
// 		m_PrevKeyState.clear();
// 		m_MouseButtonState.fill(false);
// 		m_PrevMouseButtonState.fill(false);
// 	}

// 	// --- Update ---

// 	void Input::update() 
// 	{
// 		if (!m_Window) return;

// 		// Update previous key states
// 		m_PrevKeyState = m_KeyState;

// 		// Poll all commonly used keys
// 		// (You can expand this list or use a different approach if needed)
// 		static const int commonKeys[] = {
// 			GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
// 			GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_F,
// 			GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_ALT,
// 			GLFW_KEY_ESCAPE, GLFW_KEY_ENTER, GLFW_KEY_TAB,
// 			GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
// 			GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4,
// 			GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9,
// 		};

// 		for (int key : commonKeys) 
// 		{
// 			int state = glfwGetKey(m_Window, key);

// 			m_KeyState[key] = (state == GLFW_PRESS || state == GLFW_REPEAT);
// 		}

// 		// Update previous mouse button states
// 		m_PrevMouseButtonState = m_MouseButtonState;

// 		// Poll mouse buttons
// 		for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button) 
// 		{
// 			int state = glfwGetMouseButton(m_Window, button);

// 			m_MouseButtonState[button] = (state == GLFW_PRESS);
// 		}

// 		// Update mouse position and delta
// 		m_PrevMousePos = m_MousePos;
// 		double x, y;
// 		glfwGetCursorPos(m_Window, &x, &y);
// 		m_MousePos = Vec2(static_cast<F32>(x), static_cast<F32>(y));

// 		if (m_FirstFrame) 
// 		{
// 			// Prevent huge delta on first frame
// 			m_MouseDelta = Vec2(0.0f, 0.0f);
// 			m_FirstFrame = false;
// 		} 
// 		else 
// 		{
// 			m_MouseDelta = m_MousePos - m_PrevMousePos;
// 		}

// 		// Scroll delta is reset to zero each frame (only non-zero if callback was triggered)
// 		// The callback accumulates scroll events, but we reset here for next frame
// 		// Note: m_ScrollDelta is set by the callback
// 	}

// 	// --- Keyboard ---

// 	bool Input::isKeyPressed(int key) 
// 	{
// 		auto it = m_KeyState.find(key);
	
// 		return it != m_KeyState.end() && it->second;
// 	}

// 	bool Input::wasKeyJustPressed(int key) 
// 	{
// 		auto curIt = m_KeyState.find(key);
// 		auto prevIt = m_PrevKeyState.find(key);

// 		bool current = (curIt != m_KeyState.end() && curIt->second);
// 		bool previous = (prevIt != m_PrevKeyState.end() && prevIt->second);

// 		return current && !previous;
// 	}

// 	bool Input::wasKeyJustReleased(int key) 
// 	{
// 		auto curIt = m_KeyState.find(key);
// 		auto prevIt = m_PrevKeyState.find(key);

// 		bool current = (curIt != m_KeyState.end() && curIt->second);
// 		bool previous = (prevIt != m_PrevKeyState.end() && prevIt->second);

// 		return !current && previous;
// 	}

// 	// --- Mouse Buttons ---

// 	bool Input::isMouseButtonPressed(int button) 
// 	{
// 		if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;

// 		return m_MouseButtonState[button];
// 	}

// 	bool Input::wasMouseButtonJustPressed(int button) 
// 	{
// 		if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;

// 		return m_MouseButtonState[button] && !m_PrevMouseButtonState[button];
// 	}

// 	bool Input::wasMouseButtonJustReleased(int button) 
// 	{
// 		if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
	
// 		return !m_MouseButtonState[button] && m_PrevMouseButtonState[button];
// 	}

// 	// --- Mouse Position and Delta ---

// 	Vec2 Input::mousePosition() 
// 	{
// 		return m_MousePos;
// 	}

// 	Vec2 Input::mouseDelta() 
// 	{
// 		return m_MouseDelta;
// 	}

// 	Vec2 Input::scrollDelta() 
// 	{
// 		return m_ScrollDelta;
// 	}

// 	// --- Cursor Mode ---

// 	void Input::setCursorMode(int mode) 
// 	{
// 		if (!m_Window) return;

// 		m_CursorMode = mode;
// 		glfwSetInputMode(m_Window, GLFW_CURSOR, mode);

// 		// Reset first frame flag to prevent huge delta when switching modes
// 		m_FirstFrame = true;
// 	}

// 	bool Input::isCursorDisabled() 
// 	{
// 		return m_CursorMode == GLFW_CURSOR_DISABLED;
// 	}

// 	// --- GLFW Callbacks ---

// 	void Input::scrollCallback(GLFWwindow* /*window*/, double xoffset, double yoffset) 
// 	{
// 		// Accumulate scroll delta (will be reset in update())
// 		m_ScrollDelta += Vec2(static_cast<F32>(xoffset), static_cast<F32>(yoffset));
// 	}
// }