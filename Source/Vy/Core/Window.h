#pragma once

#include <Vy/GFX/Backend/VK/VKCore.h>

#include <GLFW/glfw3.h>

#include <VyLib/VyLib.h>

namespace Vy
{
    class VyEvent;

    /**
     * @struct VyWindowData
     * 
     * @brief Holds metadata about the window such as dimensions, title, and event handling.
     */
    struct VyWindowData
	{
		String Title;
		U32    Width;
		U32    Height;

		bool   ShouldInvalidateSwapchain  = false;
		bool   ShouldUpdateFullscreenMode = false;
		bool   IsFullscreen               = false;
		bool   HasFocus                   = true;
        bool   VSync                      = false;

        bool   FramebufferResized;

        Function<void(VyEvent&)> EventCallback;

        /**
         * @brief Constructs VyWindowData with default or provided values.
         * @param title The title of the window.
         * @param width The width of the window.
         * @param height The height of the window.
         */
		VyWindowData(const String& title = "VyEngine", U32 width = 900, U32 height = 900) : 
            Title(title), 
            Width(width), 
            Height(height), 
            FramebufferResized(false) 
        {
        }
	};


    /**
     * @class VyWindow
     * @brief Encapsulates a GLFW window and manages its lifecycle.
     * 
     * This class handles window creation, event processing, and Vulkan surface creation.
     */
    class VyWindow 
    {
    public:
        
        VyWindow(const VyWindowData& data);

		VyWindow(const VyWindow&) = delete;
		VyWindow(VyWindow&&)      = delete;

		~VyWindow();

		VyWindow& operator=(const VyWindow&) = delete;
		VyWindow& operator=(VyWindow&&)      = delete;

        /**
         * @brief Gets the base GLFW window.
         * @return A pointer to the GLFWwindow instance.
         */
        VY_NODISCARD
        GLFWwindow* handle() const noexcept
        { 
            return m_WindowHandle; 
        }

        /**
         * @brief Gets the base GLFW window.
         * @return A pointer to the GLFWwindow instance.
         */
        VY_NODISCARD
        GLFWwindow* handle() noexcept
        { 
            return m_WindowHandle; 
        }

        /**
         * @brief Gets the extent (width and height) of the window for Vulkan.
         * @return A VkExtent2D structure containing the window dimensions.
         */
        VkExtent2D windowExtent() const 
        { 
            return { m_Data.Width, m_Data.Height }; 
        }


		inline U32   windowWidth()  const { return m_Data.Width; }
		inline U32   windowHeight() const { return m_Data.Height; }
		inline IVec2 windowSize()   const { return { m_Data.Width, m_Data.Height }; }
		void setVSync(bool state);
		bool isVSync() const;
        /**
         * @brief Sets the event callback function for handling window events.
         * @param callback The function to handle events.
         */
        void setEventCallback(const Function<void(VyEvent&)>& callback) 
        {
            m_Data.EventCallback = callback;
        }

        /**
         * @brief Checks if the window should close.
         * @return True if the window is set to close, false otherwise.
         */
        bool shouldClose() 
        { 
            return glfwWindowShouldClose(m_WindowHandle); 
        }
        
        /**
         * @brief Checks if the window was resized.
         * @return True if the framebuffer was resized, false otherwise.
         */
        bool wasWindowResized() 
        { 
            return m_Data.FramebufferResized; 
        }
        
        void resetWindowResizedFlag() 
        { 
            m_Data.FramebufferResized = false; 
        
        }

		inline bool shouldInvalidateSwapchain() const { return m_Data.ShouldInvalidateSwapchain; }
		inline void swapchainInvalidated() { m_Data.ShouldInvalidateSwapchain = false; }

        // This method is not immediate, it will only update the fullscreen mode on the next frame.
		void setFullscreen(bool state);
		void updateFullscreenMode();
		bool isFullscreen() const;
		bool isFocused() const;

        void pollEvents();

        /**
         * @brief Creates a Vulkan surface for the window.
         * @param instance The Vulkan instance.
         * @param surface Pointer to the created Vulkan surface.
         */
        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

    private:

        /**
         * @brief Registers the GLFW callbacks for window events.
         */
        void registerCallbacks();

        // static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

        void initWindow();

        GLFWwindow*  m_WindowHandle;
        VyWindowData m_Data;
    };
}