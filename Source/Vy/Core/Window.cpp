#include <Vy/Core/Window.h>

#include <Vy/Core/Event/Event.h>
#include <Vy/Core/Input/InputMapper.h>

namespace Vy
{
    VyWindow::VyWindow(const VyWindowData& data) : 
        m_Data{ data } 
    {
        initWindow();
    }

    
    VyWindow::~VyWindow() 
    {
        glfwDestroyWindow(m_WindowHandle);
        glfwTerminate();
    }

    
    void VyWindow::initWindow() 
    {
        VY_ASSERT(glfwInit(), "Failed to initialize glfw3");
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,  GLFW_TRUE);

        m_WindowHandle = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
        
        glfwSetWindowUserPointer(m_WindowHandle, &m_Data);

        registerCallbacks();
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
	

	void VyWindow::updateFullscreenMode()
	{
		if (m_Data.ShouldUpdateFullscreenMode)
		{
			static auto winPos  = WinInitPos(m_WindowHandle);
			static auto winSize = windowSize();

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


        glfwSetWindowCloseCallback(m_WindowHandle, [](GLFWwindow* window) 
        {
			VyWindowData& data = *(VyWindowData*) glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});

        glfwSetFramebufferSizeCallback(m_WindowHandle, [](GLFWwindow* window, int width, int height) 
        {
            VyWindowData& data = *(VyWindowData*) glfwGetWindowUserPointer(window);
            {
                data.Width              = static_cast<U32>(width);
                data.Height             = static_cast<U32>(height);
                data.FramebufferResized = true;
            }

            WindowResizeEvent event(width, height);

            data.EventCallback(event);

            VY_DEBUG("GLFW Window Framebuffer Resized");
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