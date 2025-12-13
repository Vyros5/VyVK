#pragma once

#include <VyLib/VyLib.h>
#include <Vy/Core/Input/InputCode.h>

namespace Vy
{
    enum class VyEventType
    {
        None = 0,

        // Window events
        WindowClose, 
        WindowResize, 
        WindowMoved, 
        WindowFocus, 
        WindowLostFocus,

        // Keyboard events
        KeyPress, 
        KeyRelease, 
        KeyDown,

        // Mouse events
        MouseButtonPress, 
        MouseButtonRelease, 
        MouseMove, 
        MouseScroll,
    };

    // ============================================================================================
    // MARK: VyEvent

    /**
     * @class VyEvent
     * @brief Base class for all event types.
     * 
     * The VyEvent class is the base class for all event types in the engine. 
     * It provides an interface for getting the event type, name, and string representation.
     */
    class VyEvent 
    {
    public:
        virtual ~VyEvent() = default;

        virtual VyEventType getEventType() const = 0;
        virtual String      getName()      const = 0;
        virtual String      toString()     const { return getName(); }

        bool isHandled() const { return m_Handled; }
        void markHandled() { m_Handled = true; }

    private:
        bool m_Handled = false;
    };

    // ============================================================================================
    // MARK: Dispatcher

    /**
     * @class VyEventDispatcher
     * @brief A utility class for dispatching events to appropriate handlers.
     *
     * The VyEventDispatcher class is used to dispatch events to their corresponding
     * event handlers based on the event type. It holds a reference to a VyEvent object
     * and provides a method to dispatch the event if it matches a specific type.
     */
    class VyEventDispatcher 
    {
    public:
        /**
         * @brief Constructs an VyEventDispatcher with the given event.
         * @param event The event to be dispatched.
         */
        explicit VyEventDispatcher(VyEvent& event) : 
            m_Event(event) 
        {
        }

        /**
         * @brief Dispatches the event if it matches the specified type.
         *
         * This method checks if the event type matches the specified type T.
         * If it does, it calls the provided function with the event and marks
         * the event as handled.
         *
         * @tparam T The type of the event to dispatch.
         * @tparam F The type of the function to call if the event matches.
         * @param func The function to call if the event matches the specified type.
         * @return True if the event was dispatched and handled, false otherwise.
         */
        template<typename T, typename F>
        bool dispatch(const F& func) 
        {
            // Ensure the event type matches and hasn't been handled yet
            if (m_Event.getEventType() == T::getStaticType() && !m_Event.isHandled()) 
            {
                // Cast the event to the correct type and invoke the handler.
                m_Event.markHandled();

                func(static_cast<T&>(m_Event));
                
                return true;
            }

            return false;
        }

    private:
        VyEvent& m_Event; 
    };



    // ============================================================================================
    // MARK: Window

    class WindowCloseEvent : public VyEvent 
    {
    public:
        WindowCloseEvent() = default;

        VyEventType getEventType() const override { return VyEventType::WindowClose; }
        String      getName()      const override { return "WindowClose"; }

        static VyEventType getStaticType() { return VyEventType::WindowClose; }
    };


    class WindowResizeEvent : public VyEvent 
    {
    public:
        WindowResizeEvent(U32 width, U32 height) : 
            m_Width(width), 
            m_Height(height) 
        {
        }

        U32 getWidth()  const { return m_Width;  }
        U32 getHeight() const { return m_Height; }

        VyEventType getEventType() const override { return VyEventType::WindowResize; }
        String      getName()      const override { return "WindowResize"; }

        static VyEventType getStaticType() { return VyEventType::WindowResize; }

    private:
        U32 m_Width, m_Height;
    };

    // ============================================================================================
    // MARK: Mouse Button

    class MouseButtonEvent : public VyEvent 
    {
	public:
        MouseButton getMouseButton() const { return m_Button; }
		
	protected:
		MouseButtonEvent(const MouseButton button) : 
            m_Button(button) 
        {
        }

		MouseButton m_Button;
	};

    
    class MouseButtonPressEvent : public MouseButtonEvent 
    {
    public:
        MouseButtonPressEvent(const MouseButton button) : 
            MouseButtonEvent(button) 
        {
        }

        VyEventType getEventType() const override { return VyEventType::MouseButtonPress; }
        String      getName()      const override { return "MouseButtonPress"; }

        static VyEventType getStaticType() { return VyEventType::MouseButtonPress; }
    };


    class MouseButtonReleaseEvent : public MouseButtonEvent 
    {
    public:
        MouseButtonReleaseEvent(const MouseButton button) : 
            MouseButtonEvent(button) 
        {
        }

        VyEventType getEventType() const override { return VyEventType::MouseButtonRelease; }
        String      getName()      const override { return "MouseButtonRelease"; }

        static VyEventType getStaticType() { return VyEventType::MouseButtonRelease; }
    };

    // ============================================================================================
    // MARK: Mouse

    class MouseMoveEvent : public VyEvent 
    {
    public:
        MouseMoveEvent(const double x, const double y) : 
            m_X(x), 
            m_Y(y) 
        {
        }

        double getX() const { return m_X; }
        double getY() const { return m_Y; }

        VyEventType getEventType() const override { return VyEventType::MouseMove; }
        String      getName()      const override { return "MouseMove"; }

        static VyEventType getStaticType() { return VyEventType::MouseMove; }

    private:
        double m_X, m_Y;
    };


    class MouseScrollEvent : public VyEvent 
    {
    public:
        MouseScrollEvent(const double xOffset, const double yOffset) : 
            m_XOffset(xOffset), 
            m_YOffset(yOffset) 
        {
        }

        double getXOffset() const { return m_XOffset; }
        double getYOffset() const { return m_YOffset; }

        VyEventType getEventType() const override { return VyEventType::MouseScroll; }
        String      getName()      const override { return "MouseScroll"; }

        static VyEventType getStaticType() { return VyEventType::MouseScroll; }

    private:
        double m_XOffset, m_YOffset;
    };

    // ============================================================================================
    // MARK: Keyboard

    class KeyBoardEvent : public VyEvent 
    {
    public:
        KeyCode getKeyCode() const { return m_KeyCode; }

    protected:
        KeyBoardEvent(KeyCode keyCode): m_KeyCode(keyCode) {}

        KeyCode m_KeyCode;
    };


    class KeyDownEvent : public KeyBoardEvent 
    {
    public:
        KeyDownEvent(KeyCode keyCode) : 
            KeyBoardEvent(keyCode) 
        {
        }

        VyEventType getEventType() const override { return VyEventType::KeyDown; }
        String      getName()      const override { return "KeyDown"; }

        static VyEventType getStaticType() { return VyEventType::KeyDown; }
    };


    class KeyPressEvent : public KeyBoardEvent 
    {
    public:
        KeyPressEvent(KeyCode keyCode) : 
            KeyBoardEvent(keyCode) 
        {
        }

        KeyPressEvent(KeyCode keyCode, int repeatCount) : 
            KeyBoardEvent(keyCode), 
            m_RepeatCount(repeatCount) 
        {
        }

        VyEventType getEventType() const override { return VyEventType::KeyPress; }
        String      getName()      const override { return "KeyPress"; }

        static VyEventType getStaticType() { return VyEventType::KeyPress; }

    private:
        int m_RepeatCount = 0;
    };


    class KeyReleaseEvent : public KeyBoardEvent 
    {
    public:
        KeyReleaseEvent(KeyCode keyCode) : 
            KeyBoardEvent(keyCode) 
        {
        }

        VyEventType getEventType() const override { return VyEventType::KeyRelease; }
        String      getName()      const override { return "KeyRelease"; }

        static VyEventType getStaticType() { return VyEventType::KeyRelease; }
    };

    // ============================================================================================
    
}