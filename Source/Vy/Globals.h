#pragma once

#include <VyLib/VyLib.h>

#include <limits>

#ifndef PROJECT_DIR
#	define PROJECT_DIR "C:/VKDev/VyVK"
#endif

#ifndef BUILD_DIR
#	define BUILD_DIR "C:/VKDev/VyVK/bin/build/core"
#endif

#define ENGINE_DIR PROJECT_DIR "/"
#define SHADER_DIR BUILD_DIR   "/Data/Shaders/"
#define ASSETS_DIR ENGINE_DIR  "Data/"
#define SCENES_DIR ASSETS_DIR  "Scenes/"
#define MODELS_DIR ASSETS_DIR  "Models/"
#define CUBEMAP_DIR ASSETS_DIR "Cubemap/"


namespace Vy
{ // C:\dev\Engine\Vy\Source\Vy\Core\Globals.h

	constexpr U32 DEFAULT_WIDTH { 1920 };
	constexpr U32 DEFAULT_HEIGHT{ 1080 };

	// constexpr bool ENABLE_FPS_LIMIT{ true };
	// constexpr U32  TARGET_FPS{ 144 };
	// constexpr U32 INVALID_HANDLE{ kMaxU32 };

	namespace CubeFace 
	{
		constexpr U32 RIGHT  = 0;
		constexpr U32 LEFT   = 1;
		constexpr U32 TOP    = 2;
		constexpr U32 BOTTOM = 3;
		constexpr U32 BACK   = 4;
		constexpr U32 FRONT  = 5;
	}
}