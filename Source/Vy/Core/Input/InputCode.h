#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
    enum class KeyCode 
    {
        Unknown,
        
        Backspace,       // Backspace
        Delete,          // Delete
        Tab,             // Tab
        Clear,           // Clear
        Return,          // Return
        Pause,           // Pause
        Escape,          // Escape
        Space,           // Space
        Insert,          // Insert
        Home,            // Home
        End,             // End
        PageUp,          // Page Up
        PageDown,        // Page Down

        // Keypad
        Keypad0,         // Keypad 0
        Keypad1,         // Keypad 1
        Keypad2,         // Keypad 2
        Keypad3,         // Keypad 3
        Keypad4,         // Keypad 4
        Keypad5,         // Keypad 5
        Keypad6,         // Keypad 6
        Keypad7,         // Keypad 7
        Keypad8,         // Keypad 8
        Keypad9,         // Keypad 9
        KeypadPeriod,    // Keypad .
        KeypadDivide,    // Keypad /
        KeypadMultiply,  // Keypad *
        KeypadMinus,     // Keypad -
        KeypadPlus,      // Keypad +
        KeypadEnter,     // Keypad Enter
        KeypadEquals,    // Keypad =

        // Arrow keys
        UpArrow,         // Up Arrow
        DownArrow,       // Down Arrow
        RightArrow,      // Right Arrow
        LeftArrow,       // Left Arrow

        // Function keys
        F1,              // F1
        F2,              // F2
        F3,              // F3
        F4,              // F4
        F5,              // F5
        F6,              // F6
        F7,              // F7
        F8,              // F8
        F9,              // F9
        F10,             // F10
        F11,             // F11
        F12,             // F12
        F13,             // F13
        F14,             // F14
        F15,             // F15

        // Numbers on top of keyboard
        Number0,         // 0
        Number1,         // 1
        Number2,         // 2
        Number3,         // 3
        Number4,         // 4
        Number5,         // 5
        Number6,         // 6
        Number7,         // 7
        Number8,         // 8
        Number9,         // 9

        // Letters
        A,               // A
        B,               // B
        C,               // C
        D,               // D
        E,               // E
        F,               // F
        G,               // G
        H,               // H
        I,               // I
        J,               // J
        K,               // K
        L,               // L
        M,               // M
        N,               // N
        O,               // O
        P,               // P
        Q,               // Q
        R,               // R
        S,               // S
        T,               // T
        U,               // U
        V,               // V
        W,               // W
        X,               // X
        Y,               // Y
        Z,               // Z

        // Special characters
        Exclaim,         // !
        DoubleQuote,     // "
        Hash,            // #
        Dollar,          // $
        Percent,         // %
        Ampersand,       // &
        Quote,           // '
        LeftParen,       // (
        RightParen,      // )
        Asterisk,        // *
        Plus,            // +
        Minus,           // -
        Comma,           // ,
        Period,          // .
        Slash,           // /
        Colon,           // :
        Semicolon,       // ;
        Less,            // <
        Equals,          // =
        Greater,         // >
        Question,        // ?
        At,              // @
        LeftBracket,     // [
        Backslash,       // \ 
        RightBracket,    // ]
        Caret,           // ^
        Underscore,      // _
        Backquote,       // `

        // Remaining keys
        CapsLock,        // Caps Lock
        ScrollLock,      // Scroll Lock
        NumLock,         // Num Lock
        PrintScreen,     // Print Screen
        PauseBreak,      // Pause/Break
        Menu,            // Menu
        LeftShift,       // Left Shift
        RightShift,      // Right Shift
        LeftControl,     // Left Control
        RightControl,    // Right Control
        LeftAlt,         // Left Alt
        RightAlt,        // Right Alt
        LeftMeta,        // Left Meta (Windows/Command key)
        RightMeta,       // Right Meta (Windows/Command key)
        LeftSuper,       // Left Super (Windows/Command key)
        RightSuper,      // Right Super (Windows/Command key)
        LeftHyper,       // Left Hyper
        RightHyper,      // Right Hyper
        LeftFn,          // Left Fn
        RightFn,         // Right Fn
    };

    enum class MouseButton 
    {
        Unknown,

        Button0,	      // The Left (or primary) mouse button.
        Button1,	      // Right mouse button (or secondary mouse button).
        Button2,	      // Middle mouse button (or third button).
        Button3,	      // Additional (fourth) mouse button.
        Button4,	      // Additional (fifth) mouse button.
        Button5,	      // Additional (or sixth) mouse button.
        Button6,	      // Additional (or seventh) mouse button.
        Button7,	      // Additional (or eighth) mouse button.
        Button8,          // Additional (or ninth) mouse button.
    };

    constexpr MouseButton LeftMouseButton   = MouseButton::Button0;
    constexpr MouseButton RightMouseButton  = MouseButton::Button1;
    constexpr MouseButton MiddleMouseButton = MouseButton::Button2;
}

// namespace Vy
// {
//     using KeyCode = U16;

// 	namespace Key
// 	{
// 		enum : KeyCode
// 		{
// 			// From glfw3.h
// 			Space               = 32,
// 			Apostrophe          = 39, /* ' */
// 			Comma               = 44, /* , */
// 			Minus               = 45, /* - */
// 			Period              = 46, /* . */
// 			Slash               = 47, /* / */

// 			Num0                = 48, /* 0 */
// 			Num1                = 49, /* 1 */
// 			Num2                = 50, /* 2 */
// 			Num3                = 51, /* 3 */
// 			Num4                = 52, /* 4 */
// 			Num5                = 53, /* 5 */
// 			Num6                = 54, /* 6 */
// 			Num7                = 55, /* 7 */
// 			Num8                = 56, /* 8 */
// 			Num9                = 57, /* 9 */

// 			Semicolon           = 59, /* ; */
// 			Equal               = 61, /* = */

// 			A                   = 65,
// 			B                   = 66,
// 			C                   = 67,
// 			D                   = 68,
// 			E                   = 69,
// 			F                   = 70,
// 			G                   = 71,
// 			H                   = 72,
// 			I                   = 73,
// 			J                   = 74,
// 			K                   = 75,
// 			L                   = 76,
// 			M                   = 77,
// 			N                   = 78,
// 			O                   = 79,
// 			P                   = 80,
// 			Q                   = 81,
// 			R                   = 82,
// 			S                   = 83,
// 			T                   = 84,
// 			U                   = 85,
// 			V                   = 86,
// 			W                   = 87,
// 			X                   = 88,
// 			Y                   = 89,
// 			Z                   = 90,

// 			LeftBracket         = 91,  /* [ */
// 			Backslash           = 92,  /* \ */
// 			RightBracket        = 93,  /* ] */
// 			GraveAccent         = 96,  /* ` */

// 			World1              = 161, /* non-US #1 */
// 			World2              = 162, /* non-US #2 */

// 			/* Function keys */
// 			Escape              = 256,
// 			Enter               = 257,
// 			Tab                 = 258,
// 			Backspace           = 259,
// 			Insert              = 260,
// 			Delete              = 261,
// 			Right               = 262,
// 			Left                = 263,
// 			Down                = 264,
// 			Up                  = 265,
// 			PageUp              = 266,
// 			PageDown            = 267,
// 			Home                = 268,
// 			End                 = 269,
// 			CapsLock            = 280,
// 			ScrollLock          = 281,
// 			NumLock             = 282,
// 			PrintScreen         = 283,
// 			Pause               = 284,
// 			F1                  = 290,
// 			F2                  = 291,
// 			F3                  = 292,
// 			F4                  = 293,
// 			F5                  = 294,
// 			F6                  = 295,
// 			F7                  = 296,
// 			F8                  = 297,
// 			F9                  = 298,
// 			F10                 = 299,
// 			F11                 = 300,
// 			F12                 = 301,
// 			F13                 = 302,
// 			F14                 = 303,
// 			F15                 = 304,
// 			F16                 = 305,
// 			F17                 = 306,
// 			F18                 = 307,
// 			F19                 = 308,
// 			F20                 = 309,
// 			F21                 = 310,
// 			F22                 = 311,
// 			F23                 = 312,
// 			F24                 = 313,
// 			F25                 = 314,

// 			/* Keypad */
// 			KP0                 = 320,
// 			KP1                 = 321,
// 			KP2                 = 322,
// 			KP3                 = 323,
// 			KP4                 = 324,
// 			KP5                 = 325,
// 			KP6                 = 326,
// 			KP7                 = 327,
// 			KP8                 = 328,
// 			KP9                 = 329,
// 			KPDecimal           = 330,
// 			KPDivide            = 331,
// 			KPMultiply          = 332,
// 			KPSubtract          = 333,
// 			KPAdd               = 334,
// 			KPEnter             = 335,
// 			KPEqual             = 336,

// 			LeftShift           = 340,
// 			LeftControl         = 341,
// 			LeftAlt             = 342,
// 			LeftSuper           = 343,
// 			RightShift          = 344,
// 			RightControl        = 345,
// 			RightAlt            = 346,
// 			RightSuper          = 347,
// 			Menu                = 348,

// 			MAX,
// 		};
// 	}


//     using MouseCode = U16;

//     namespace Mouse
//     {
//         enum : MouseCode
//         {
//             Button0                = 0,
//             Button1                = 1,
//             Button2                = 2,
//             Button3                = 3,
//             Button4                = 4,
//             Button5                = 5,
//             Button6                = 6,
//             Button7                = 7,

//             ButtonLast             = Button7,
//             ButtonLeft             = Button0,
//             ButtonRight            = Button1,
//             ButtonMiddle           = Button2,

//             MAX = 8,
//         };
//     } 
// }