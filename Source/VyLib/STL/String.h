#pragma once

#include <string>
#include <string_view>

namespace Vy
{
    using CString     = const char*;    // C String Type (const char*)
    using WCString    = const wchar_t*; // C Wide-String Type (const wchar_t*)

    using String      = std::string;
    using StringView  = std::string_view;

    using WString     = std::wstring;
    using WStringView = std::wstring_view;

    // ============================================================================================
    // Custom String Types:

}