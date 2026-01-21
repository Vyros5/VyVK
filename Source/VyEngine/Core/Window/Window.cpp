#include <VyEngine/Core/Window/Window.h>

// #include <VyEngine/Core/Event/Event.h>
// #include <VyEngine/Core/Input/InputMapper.h>
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <thread>

#ifdef __linux__
#include <X11/Xlib.h>
#endif

// Small helpers to keep initWindow simple and readable.
namespace window_detail 
{
	#ifdef __linux__
	// Try to get the global cursor position via X11 (useful for XWayland).
	bool tryGetXCursorPosition(int& outX, int& outY)
	{
		if (!getenv("DISPLAY")) return false;
		::Display* dpy = XOpenDisplay(nullptr);
		if (!dpy) return false;

		::Window     root  = DefaultRootWindow(dpy);
		::Window     ret   = 0;
		::Window     child = 0;
		int          rootx = 0;
		int          rooty = 0;
		int          winx  = 0;
		int          winy  = 0;
		unsigned int mask  = 0;
		const Bool   ok    = XQueryPointer(dpy, root, &ret, &child, &rootx, &rooty, &winx, &winy, &mask);
		XCloseDisplay(dpy);
		if (!ok) return false;
		outX = rootx;
		outY = rooty;
		return true;
	}
	#endif

	// Pick a monitor containing the cursor. If haveCursor is false, returns
	// nullptr.
	GLFWmonitor* pickMonitorForCursor(GLFWmonitor** monitors, int monitorCount, int cursorX, int cursorY)
	{
		for (int i = 0; i < monitorCount; ++i)
		{
			int mx = 0;
			int my = 0;
			glfwGetMonitorPos(monitors[i], &mx, &my);
		
			const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
			if (!mode) continue;
		
			int mw = mode->width;
			int mh = mode->height;
		
			if (cursorX >= mx && cursorX < mx + mw && cursorY >= my && cursorY < my + mh)
			{
				return monitors[i];
			}
		}

		return nullptr;
	}

	// Choose a target monitor given cursor availability; returns primary
	// monitor as fallback.
	GLFWmonitor* chooseTargetMonitor(bool haveCursor, int cursorX, int cursorY)
	{
		int           monitorCount = 0;
		GLFWmonitor** monitors     = glfwGetMonitors(&monitorCount);
		if (haveCursor)
		{
			GLFWmonitor* pFound = pickMonitorForCursor(monitors, monitorCount, cursorX, cursorY);

			if (pFound) 
			{
				return pFound;
			}
		}

		return glfwGetPrimaryMonitor();
	}

	// Wait for the window position to stabilize or become non-zero. Returns
	// last pos.
	void waitForWindowStabilize(GLFWwindow* window, int& outX, int& outY)
	{
		int       prevX    = INT_MIN;
		int       prevY    = INT_MIN;
		const int maxIters = 100; // ~1s
		
		for (int i = 0; i < maxIters; ++i)
		{
			glfwPollEvents();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			glfwGetWindowPos(window, &outX, &outY);

			if ((outX != 0 || outY != 0) && outX == prevX && outY == prevY) break;
			
			prevX = outX;
			prevY = outY;
		}
	}

	// Request centering on the given monitor (best-effort).
	void centerWindowOnMonitor(GLFWwindow* window, GLFWmonitor* monitor, int width, int height)
	{
		if (!monitor) return;

		int mx = 0;
		int my = 0;
		glfwGetMonitorPos(monitor, &mx, &my);
		
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		if (!mode) return;
		int xpos = mx + (mode->width - width) / 2;
		int ypos = my + (mode->height - height) / 2;
		
		// std::cout << "[" << BLUE << "Window" << RESET << "]" << YELLOW << (glfwGetMonitorName(monitor) ? glfwGetMonitorName(monitor) : "unknown") << "' at ("
				// << xpos << ", " << ypos << ")" << RESET << "\n";
		glfwSetWindowPos(window, xpos, ypos);
	}
} // namespace window_detail



namespace Vy
{
    VyWindow::VyWindow(const VyWindowData& data) : 
        m_Data{ data } 
    {
        initWindow();
    }

    
    VyWindow::~VyWindow() 
    {
		if (m_WindowHandle)
		{
			glfwDestroyWindow(m_WindowHandle);
		
			m_WindowHandle = nullptr;
		}

		if (m_GlfwInitialized)
		{
			glfwTerminate();
		
			m_GlfwInitialized = false;
		}
    }

    
    void VyWindow::initWindow() 
    {
		if (m_GlfwInitialized) return;

        VY_ASSERT(glfwInit(), "Failed to initialize glfw3");

		m_GlfwInitialized = true;
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,  GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE,    GLFW_FALSE);

		// Try to pick the monitor where the user likely wants the window.
		int  cursorX    = 0;
		int  cursorY    = 0;
		bool haveCursor = false;

	#ifdef __linux__
		haveCursor = window_detail::tryGetXCursorPosition(cursorX, cursorY);
	#endif

		GLFWmonitor* pTargetMonitor = window_detail::chooseTargetMonitor(haveCursor, cursorX, cursorY);

		// Create the window (hidden)
        m_WindowHandle = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
        
        glfwSetWindowUserPointer(m_WindowHandle, &m_Data);

        registerCallbacks();

		// If we have a target monitor, compute centered position and request
		// it. Note: on Wayland compositors (Hyperland) the compositor may
		// ignore this request.
		if (pTargetMonitor)
		{
			int mx = 0;
			int my = 0;

			glfwGetMonitorPos(pTargetMonitor, &mx, &my);
			const GLFWvidmode* mode = glfwGetVideoMode(pTargetMonitor);

			if (mode)
			{
				int xpos = mx + (mode->width  - m_Data.Width ) / 2;
				int ypos = my + (mode->height - m_Data.Height) / 2;

				auto monitorName = glfwGetMonitorName(pTargetMonitor);
				// std::cout << "[ " << BLUE << "Window" << RESET << " ] " << YELLOW << (monitorName ? monitorName : "unknown") << BLUE << " position (" << xpos << ", "
				// 		<< ypos << ")" << RESET << "\n";
				glfwSetWindowPos(m_WindowHandle, xpos, ypos);
			}
		}

		// Wayland compositors sometimes ignore our initial placement request if
		// made immediately after creation, so wait briefly for the compositor
		// to react.
		int posX = 0;
		int posY = 0;
		window_detail::waitForWindowStabilize(m_WindowHandle, posX, posY);

		// Show the window now that we've attempted to position it.
		glfwShowWindow(m_WindowHandle);

		// If the compositor still left us at (0, 0), try centering manually.
		if (pTargetMonitor)
		{
			glfwGetWindowPos(m_WindowHandle, &posX, &posY);

			if (posX == 0 && posY == 0)
			{
				window_detail::centerWindowOnMonitor(m_WindowHandle, pTargetMonitor, m_Data.Width, m_Data.Height);
			}
		}
    }


	void VyWindow::pollEvents()
	{
		glfwPollEvents();
	}

        
    void VyWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) 
    {
        if (glfwCreateWindowSurface(instance, m_WindowHandle, nullptr, surface) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create window surface");
        }
    }


	static IVec2 WinInitPos(GLFWwindow* win)
	{
		int x, y;
		glfwGetWindowPos(win, &x, &y);
		return { x, y };
	}


	void VyWindow::setFullscreen(bool state)
	{
		m_Data.ShouldUpdateFullscreenMode = true;
		m_Data.IsFullscreen               = state;
	}
	
	void VyWindow::setCursorVisible(bool visible)
	{
		m_CursorVisible = visible;
		glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}

	void VyWindow::toggleCursor()
	{
		setCursorVisible( !m_CursorVisible );
	}

	void VyWindow::updateFullscreenMode()
	{
		if (m_Data.ShouldUpdateFullscreenMode)
		{
			static auto winPos  = WinInitPos(m_WindowHandle);
			static auto winSize = this->windowSize();

			if (m_Data.IsFullscreen)
			{
				const auto monitor = glfwGetPrimaryMonitor();
				const auto mode    = glfwGetVideoMode(monitor);

				// Store windows pos data to restore later
				glfwGetWindowPos(m_WindowHandle, &winPos.x, &winPos.y);
				glfwGetWindowSize(m_WindowHandle, &winSize.x, &winSize.y);

				glfwSetWindowMonitor(m_WindowHandle, monitor, 0, 0, mode->width, mode->height, isVSync() ? mode->refreshRate : 0);
			}
			else
			{
				glfwSetWindowMonitor(m_WindowHandle, nullptr, winPos.x, winPos.y, winSize.x, winSize.y, 0);
			}

			m_Data.ShouldInvalidateSwapchain  = true;
			m_Data.ShouldUpdateFullscreenMode = false;
		}
	}

	
	bool VyWindow::isFullscreen() const
	{
		return m_Data.IsFullscreen;
	}


	bool VyWindow::isFocused() const
	{
		return m_Data.HasFocus;
	}


	void VyWindow::setVSync(bool state)
	{
		glfwSwapInterval(state);

		m_Data.VSync                     = state;
		m_Data.ShouldInvalidateSwapchain = true;
	}


	bool VyWindow::isVSync() const
	{
		return m_Data.VSync;
	}


    void VyWindow::registerCallbacks()
    {
        // glfwSetFramebufferSizeCallback(m_WindowHandle, framebufferResizeCallback);


        // glfwSetWindowCloseCallback(m_WindowHandle, [](GLFWwindow* window) 
        // {
		// 	VyWindowData& data = *(VyWindowData*) glfwGetWindowUserPointer(window);

		// 	WindowCloseEvent event;
		// 	data.EventCallback(event);
		// });

        glfwSetFramebufferSizeCallback(m_WindowHandle, [](GLFWwindow* pWindow, int width, int height) 
        {
            VyWindowData& data = *(VyWindowData*)glfwGetWindowUserPointer(pWindow);
            {
                data.Width              = static_cast<U32>(width);
                data.Height             = static_cast<U32>(height);
                data.FramebufferResized = true;
            }

            // WindowResizeEvent event(width, height);

            // data.EventCallback(event);

            // VY_TRACE_TAG("VyWindow", "GLFW Window Framebuffer Resized");
        });

        // glfwSetKeyCallback(m_WindowHandle, [](GLFWwindow* window, int glfwKey, int scancode, int action, int mods) 
        // {
		// 	VyWindowData& data = *(VyWindowData*) glfwGetWindowUserPointer(window);

        //     KeyCode key = mapGLFWKey(glfwKey);

		// 	switch (action) 
        //     {
		// 		case GLFW_PRESS:
		// 		{
		// 			KeyPressEvent event(key);
		// 			data.EventCallback(event);
		// 			break;
		// 		}
		// 		case GLFW_RELEASE:
		// 		{
		// 			KeyReleaseEvent event(key);
		// 			data.EventCallback(event);
		// 			break;
		// 		}
		// 		case GLFW_REPEAT:
		// 		{
		// 			KeyPressEvent event(key, 1);
		// 			data.EventCallback(event);
		// 			break;
		// 		}
		// 	}
		// });

        // glfwSetCharCallback(m_WindowHandle, [](GLFWwindow* window, unsigned int glfwKey)
		// {
		// 	VyWindowData& data = *(VyWindowData*) glfwGetWindowUserPointer(window);

        //     KeyCode key = mapGLFWKey(glfwKey);

		// 	KeyDownEvent event(key);
		// 	data.EventCallback(event);
		// });

		// glfwSetMouseButtonCallback(m_WindowHandle, [](GLFWwindow* window, int glfwButton, int action, int mods)
		// {
		// 	VyWindowData& data = *(VyWindowData*) glfwGetWindowUserPointer(window);
            
		// 	MouseButton button = mapGLFWMouseButton(glfwButton);

		// 	switch (action)
		// 	{
		// 		case GLFW_PRESS:
		// 		{
		// 			MouseButtonPressEvent event(button);
		// 			data.EventCallback(event);
		// 			break;
		// 		}
		// 		case GLFW_RELEASE:
		// 		{
		// 			MouseButtonReleaseEvent event(button);
		// 			data.EventCallback(event);
		// 			break;
		// 		}
		// 	}
		// });

		// glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow* window, double xOffset, double yOffset)
		// {
		// 	VyWindowData& data = *(VyWindowData*) glfwGetWindowUserPointer(window);

		// 	MouseScrollEvent event(xOffset, yOffset);
		// 	data.EventCallback(event);
		// });

		// glfwSetCursorPosCallback(m_WindowHandle, [](GLFWwindow* window, double xPos, double yPos)
		// {
		// 	VyWindowData& data = *(VyWindowData*) glfwGetWindowUserPointer(window);

		// 	MouseMoveEvent event(xPos, yPos);
		// 	data.EventCallback(event);
		// });
    }


    // void VyWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) 
    // {
    //     auto vyWindow = reinterpret_cast<VyWindow*>(glfwGetWindowUserPointer(window));
    //     {
    //         vyWindow->m_Data.FramebufferResized = true;
    //         vyWindow->m_Data.Width              = width;
    //         vyWindow->m_Data.Height             = height;
    //     }
    // }
}