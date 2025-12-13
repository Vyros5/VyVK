#pragma once

#include <VyLib/VyLib.h>

#ifdef VY_PLATFORM_WINDOWS
#   include <VyVK/Core/Platform/Platform.Win32.h>
#else
#   error "Unsupported platform!"
#endif