#pragma once

#include <Vy/Core/Window.h>


namespace Vy
{
	class VyInput
	{
	public:
		enum KeyEvent;
		enum Modifier;
		enum Key;
		enum MouseButton;

		VyInput(const VyWindow& window);

		VyInput(const VyInput&) = delete;
		VyInput(VyInput&&)      = delete;
		
		~VyInput();

		VyInput& operator=(const VyInput&) = delete;
		VyInput& operator=(VyInput&&)      = delete;

		/**
		 * @brief Acces to the instance of VyInput.
		 * 
		 * @note Make sure VyInput was initialized before calling this.
		 * 
		 * @return A reference to the instance of VyInput.
		 */
		static VyInput& get();

		/**
		 * @brief Returns the state of a key.
		 * 
		 * @param key Keycode of the key.
		 * 
		 * @return True if the key is pressed otherwise false.
		 */
		bool keyPressed(Key key);


		/**
		 * @brief Returns the state of a button.
		 * 
		 * @param button Buttoncode of the mousebutton.
		 * 
		 * @return True if the button is pressed otherwise false.
		 */
		bool mouseButtonPressed(MouseButton button);

		/**
		 * @brief Retrieves the mouse cursor position.
		 * 
		 * @return A Vec2 with the cursor position.
		 */
		Vec2 cursorPosition();


		/**
		 * @brief Set the Input Mode.
		 * 
		 * @param mode  The mode to change.
		 * @param value The new value for the mode.
		 */
		void setInputMode(int mode, int value);


		/**
		 * @brief Binds a function to a key.
		 * 
		 * @tparam T       Type of the class that contains the function.
		 * @param instance Pointer to the instance of the class.
		 * @param func     Function to bind.
		 * @param key      Key to bind the function to (A, B, C, etc...)
		 * @param event    Event which triggers the function (press, release, etc...)
		 * @param mod      Any modifier keys that should be pressed (shift, control, etc...)
		 * 
		 * @note The function will be executed in glfwPollEvents.
		 */
		template<typename T>
		void bind(T* instance, void (T::* func)(), Key key, KeyEvent event = Press, Modifier mod = None)
		{
			m_KeyBindings[key].push_back({ std::bind(func, instance), event, mod });
		}

	private:
		struct Binding
		{
			Function<void()> Func;
			KeyEvent         Event;
			Modifier         Mods;
		};

		/// @brief Calls the bound functions for the key.
		void glfwKeyCallback(int key, int scancode, KeyEvent action, Modifier mods);

		static VyInput*                 s_Instance;

		GLFWwindow*                     m_Window = nullptr;
		THashMap<int, TVector<Binding>> m_KeyBindings;

		/// Definitions of Keycodes, events and modifiers
	public:
		enum KeyEvent
		{
			Press   = GLFW_PRESS,
			Release = GLFW_RELEASE,
			Repeat  = GLFW_REPEAT
		};

		/// @note Can be combined with bitwise OR
		enum Modifier
		{
			None     = 0,
			Shift    = GLFW_MOD_SHIFT,
			Control  = GLFW_MOD_CONTROL,
			Alt      = GLFW_MOD_ALT,
			Super    = GLFW_MOD_SUPER,
			CapsLock = GLFW_MOD_CAPS_LOCK,
			NumLock  = GLFW_MOD_NUM_LOCK
		};

		enum MouseButton
		{
			Mouse1      = GLFW_MOUSE_BUTTON_1,
			Mouse2      = GLFW_MOUSE_BUTTON_2,
			Mouse3      = GLFW_MOUSE_BUTTON_3,
			Mouse4      = GLFW_MOUSE_BUTTON_4,
			Mouse5      = GLFW_MOUSE_BUTTON_5,
			Mouse6      = GLFW_MOUSE_BUTTON_6,
			Mouse7      = GLFW_MOUSE_BUTTON_7,
			Mouse8      = GLFW_MOUSE_BUTTON_8,
			MouseLast   = GLFW_MOUSE_BUTTON_LAST,
			MouseLeft   = GLFW_MOUSE_BUTTON_LEFT,
			MouseRight  = GLFW_MOUSE_BUTTON_RIGHT,
			MouseMiddle = GLFW_MOUSE_BUTTON_MIDDLE
		};

		enum Key
		{
			Unknown = GLFW_KEY_UNKNOWN,

			Space      = GLFW_KEY_SPACE,
			Apostrophe = GLFW_KEY_APOSTROPHE,
			Comma      = GLFW_KEY_COMMA,
			Minus      = GLFW_KEY_MINUS,
			Period     = GLFW_KEY_PERIOD,
			Slash      = GLFW_KEY_SLASH,
			Zero       = GLFW_KEY_0,
			One        = GLFW_KEY_1,
			Two        = GLFW_KEY_2,
			Three      = GLFW_KEY_3,
			Four       = GLFW_KEY_4,
			Five       = GLFW_KEY_5,
			Six        = GLFW_KEY_6,
			Seven      = GLFW_KEY_7,
			Eight      = GLFW_KEY_8,
			Nine       = GLFW_KEY_9,
			Semicolon  = GLFW_KEY_SEMICOLON,
			Equal      = GLFW_KEY_EQUAL,

			A = GLFW_KEY_A,
			B = GLFW_KEY_B,
			C = GLFW_KEY_C,
			D = GLFW_KEY_D,
			E = GLFW_KEY_E,
			F = GLFW_KEY_F,
			G = GLFW_KEY_G,
			H = GLFW_KEY_H,
			I = GLFW_KEY_I,
			J = GLFW_KEY_J,
			K = GLFW_KEY_K,
			L = GLFW_KEY_L,
			M = GLFW_KEY_M,
			N = GLFW_KEY_N,
			O = GLFW_KEY_O,
			P = GLFW_KEY_P,
			Q = GLFW_KEY_Q,
			R = GLFW_KEY_R,
			S = GLFW_KEY_S,
			T = GLFW_KEY_T,
			U = GLFW_KEY_U,
			V = GLFW_KEY_V,
			W = GLFW_KEY_W,
			X = GLFW_KEY_X,
			Y = GLFW_KEY_Y,
			Z = GLFW_KEY_Z,

			LeftBracket  = GLFW_KEY_LEFT_BRACKET,
			Backslash    = GLFW_KEY_BACKSLASH,
			RightBracket = GLFW_KEY_RIGHT_BRACKET,
			GraveAccent  = GLFW_KEY_GRAVE_ACCENT,
			World1       = GLFW_KEY_WORLD_1,
			World2       = GLFW_KEY_WORLD_2,
			Escape       = GLFW_KEY_ESCAPE,
			Enter        = GLFW_KEY_ENTER,
			Tab          = GLFW_KEY_TAB,
			Backspace    = GLFW_KEY_BACKSPACE,
			Insert       = GLFW_KEY_INSERT,
			Delete       = GLFW_KEY_DELETE,
			Right        = GLFW_KEY_RIGHT,
			Left         = GLFW_KEY_LEFT,
			Down         = GLFW_KEY_DOWN,
			Up           = GLFW_KEY_UP,
			PageUp       = GLFW_KEY_PAGE_UP,
			PageDown     = GLFW_KEY_PAGE_DOWN,
			Home         = GLFW_KEY_HOME,
			End          = GLFW_KEY_END,
			CapsLockKey  = GLFW_KEY_CAPS_LOCK,    // CapsLock is also valid
			ScrollLock   = GLFW_KEY_SCROLL_LOCK,
			NumLockKey   = GLFW_KEY_NUM_LOCK,      // NumLock is also valid
			PrintScreen  = GLFW_KEY_PRINT_SCREEN,
			Pause        = GLFW_KEY_PAUSE,

			F1  = GLFW_KEY_F1,
			F2  = GLFW_KEY_F2,
			F3  = GLFW_KEY_F3,
			F4  = GLFW_KEY_F4,
			F5  = GLFW_KEY_F5,
			F6  = GLFW_KEY_F6,
			F7  = GLFW_KEY_F7,
			F8  = GLFW_KEY_F8,
			F9  = GLFW_KEY_F9,
			F10 = GLFW_KEY_F10,
			F11 = GLFW_KEY_F11,
			F12 = GLFW_KEY_F12,
			F13 = GLFW_KEY_F13,
			F14 = GLFW_KEY_F14,
			F15 = GLFW_KEY_F15,
			F16 = GLFW_KEY_F16,
			F17 = GLFW_KEY_F17,
			F18 = GLFW_KEY_F18,
			F19 = GLFW_KEY_F19,
			F20 = GLFW_KEY_F20,
			F21 = GLFW_KEY_F21,
			F22 = GLFW_KEY_F22,
			F23 = GLFW_KEY_F23,
			F24 = GLFW_KEY_F24,
			F25 = GLFW_KEY_F25,

			KeyPad0        = GLFW_KEY_KP_0,
			KeyPad1        = GLFW_KEY_KP_1,
			KeyPad2        = GLFW_KEY_KP_2,
			KeyPad3        = GLFW_KEY_KP_3,
			KeyPad4        = GLFW_KEY_KP_4,
			KeyPad5        = GLFW_KEY_KP_5,
			KeyPad6        = GLFW_KEY_KP_6,
			KeyPad7        = GLFW_KEY_KP_7,
			KeyPad8        = GLFW_KEY_KP_8,
			KeyPad9        = GLFW_KEY_KP_9,
			KeyPadDecimal  = GLFW_KEY_KP_DECIMAL,
			KeyPadDivide   = GLFW_KEY_KP_DIVIDE,
			KeyPadMultiply = GLFW_KEY_KP_MULTIPLY,
			KeyPadSubtract = GLFW_KEY_KP_SUBTRACT,
			KeyPadAdd      = GLFW_KEY_KP_ADD,
			KeyPadEnter    = GLFW_KEY_KP_ENTER,
			KeyPadEqual    = GLFW_KEY_KP_EQUAL,

			LeftShift    = GLFW_KEY_LEFT_SHIFT,
			LeftControl  = GLFW_KEY_LEFT_CONTROL,
			LeftAlt      = GLFW_KEY_LEFT_ALT,
			LeftSuper    = GLFW_KEY_LEFT_SUPER,
			RightShift   = GLFW_KEY_RIGHT_SHIFT,
			RightControl = GLFW_KEY_RIGHT_CONTROL,
			RightAlt     = GLFW_KEY_RIGHT_ALT,
			RightSuper   = GLFW_KEY_RIGHT_SUPER,

			Menu     = GLFW_KEY_MENU,
			MaxValue = GLFW_KEY_LAST
		};
	};
}



namespace Vy
{
	// /**
	//  * Input - Centralized input state tracker
	//  *
	//  * Design principles:
	//  * - Poll GLFW state once per frame (at start of frame)
	//  * - All systems query Input instead of calling GLFW directly
	//  * - Provides delta values for mouse movement
	//  * - Frame-based input detection (pressed this frame, released this frame)
	//  * - Thread-safe: Only update() should be called from main thread
	//  *
	//  * Usage:
	//  *   Input::init(window);
	//  *
	//  *   // In main loop:
	//  *   Input::update();
	//  *
	//  *   // In systems:
	//  *   if (Input::isKeyPressed(GLFW_KEY_W)) { ... }
	//  *   if (Input::wasKeyJustPressed(GLFW_KEY_SPACE)) { ... }
	//  *   Vec2 mouseDelta = Input::mouseDelta();
	//  */
	// class Input 
	// {
	// public:
	// 	// --- Initialization ---

	// 	/**
	// 	 * Initialize input system with GLFW window
	// 	 *
	// 	 * @param window GLFW window handle
	// 	 */
	// 	static void init(GLFWwindow* window);

	// 	/**
	// 	 * Update input state (call once per frame at start of frame)
	// 	 * Polls GLFW for current key/mouse state and computes deltas
	// 	 */
	// 	static void update();

	// 	/**
	// 	 * Cleanup input system
	// 	 */
	// 	static void cleanup();

	// 	// --- Keyboard ---

	// 	/**
	// 	 * Check if key is currently pressed
	// 	 *
	// 	 * @param key GLFW key code (e.g., GLFW_KEY_W)
	// 	 * @return true if key is currently down
	// 	 */
	// 	static bool isKeyPressed(int key);

	// 	/**
	// 	 * Check if key was just pressed this frame
	// 	 *
	// 	 * @param key GLFW key code
	// 	 * @return true if key transitioned from up to down this frame
	// 	 */
	// 	static bool wasKeyJustPressed(int key);

	// 	/**
	// 	 * Check if key was just released this frame
	// 	 *
	// 	 * @param key GLFW key code
	// 	 * @return true if key transitioned from down to up this frame
	// 	 */
	// 	static bool wasKeyJustReleased(int key);

	// 	// --- Mouse Buttons ---

	// 	/**
	// 	 * Check if mouse button is currently pressed
	// 	 *
	// 	 * @param button GLFW mouse button code (e.g., GLFW_MOUSE_BUTTON_LEFT)
	// 	 * @return true if button is currently down
	// 	 */
	// 	static bool isMouseButtonPressed(int button);

	// 	/**
	// 	 * Check if mouse button was just pressed this frame
	// 	 *
	// 	 * @param button GLFW mouse button code
	// 	 * @return true if button transitioned from up to down this frame
	// 	 */
	// 	static bool wasMouseButtonJustPressed(int button);

	// 	/**
	// 	 * Check if mouse button was just released this frame
	// 	 *
	// 	 * @param button GLFW mouse button code
	// 	 * @return true if button transitioned from down to up this frame
	// 	 */
	// 	static bool wasMouseButtonJustReleased(int button);

	// 	// --- Mouse Position and Delta ---

	// 	/**
	// 	 * Get current mouse position in screen coordinates
	// 	 *
	// 	 * @return (x, y) position where (0,0) is top-left
	// 	 */
	// 	static Vec2 mousePosition();

	// 	/**
	// 	 * Get mouse movement delta since last frame
	// 	 *
	// 	 * @return (dx, dy) movement in pixels
	// 	 */
	// 	static Vec2 mouseDelta();

	// 	/**
	// 	 * Get mouse scroll wheel delta since last frame
	// 	 *
	// 	 * @return (x_offset, y_offset) where y_offset is typical vertical scroll
	// 	 */
	// 	static Vec2 scrollDelta();

	// 	// --- Cursor Mode ---

	// 	/**
	// 	 * Set cursor mode
	// 	 *
	// 	 * @param mode GLFW cursor mode:
	// 	 *   GLFW_CURSOR_NORMAL:   Cursor visible and behaves normally
	// 	 *   GLFW_CURSOR_HIDDEN:   Cursor invisible but behaves normally
	// 	 *   GLFW_CURSOR_DISABLED: Cursor hidden and locked for FPS-style camera
	// 	 */
	// 	static void setCursorMode(int mode);

	// 	/**
	// 	 * Check if cursor is disabled (locked for FPS camera)
	// 	 */
	// 	static bool isCursorDisabled();

	// private:
	// 	// GLFW window handle
	// 	static GLFWwindow* m_Window;

	// 	// Keyboard state (current and previous frame)
	// 	static THashMap<int, bool> m_KeyState;
	// 	static THashMap<int, bool> m_PrevKeyState;

	// 	// Mouse button state (current and previous frame)
	// 	static TArray<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_MouseButtonState;
	// 	static TArray<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_PrevMouseButtonState;

	// 	// Mouse position
	// 	static Vec2 m_MousePos;
	// 	static Vec2 m_PrevMousePos;
	// 	static Vec2 m_MouseDelta;

	// 	// Scroll wheel
	// 	static Vec2 m_ScrollDelta;

	// 	// Cursor mode
	// 	static int m_CursorMode;

	// 	// First frame flag (to prevent huge mouse delta on first frame)
	// 	static bool m_FirstFrame;

	// 	// GLFW callbacks
	// 	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	// 	// Prevent instantiation
	// 	Input() = delete;
	// };
}