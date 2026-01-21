#pragma once

#include <VyEngine/VK/Core/VKCore.h>

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
		TString Title;
		U32     Width;
		U32     Height;

		bool    ShouldInvalidateSwapchain  = false;
		bool    ShouldUpdateFullscreenMode = false;
		bool    IsFullscreen               = false;
		bool    HasFocus                   = true;
        bool    VSync                      = false;

        bool    FramebufferResized;

        TFunction<void(VyEvent&)> EventCallback;

        /**
         * @brief Constructs VyWindowData with default or provided values.
         * @param title The title of the window.
         * @param width The width of the window.
         * @param height The height of the window.
         */
		VyWindowData(const TString& title = "VyEngine", U32 width = 900, U32 height = 900) : 
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

        void waitEvents()
        {
            glfwWaitEvents();
        }

        void getFramebufferSize(int& outWidth, int& outHeight) const 
        {
            glfwGetFramebufferSize(m_WindowHandle, &outWidth, &outHeight);
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
        void setEventCallback(const TFunction<void(VyEvent&)>& callback) 
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
        void setCursorVisible(bool visible);
        void toggleCursor();
        bool isCursorVisible() const { return m_CursorVisible; }

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
    
        bool m_GlfwInitialized = false;
        bool m_CursorVisible    = true;
    };
}