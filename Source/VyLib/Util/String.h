#pragma once

#include <VyLib/STL/String.h>
#include <VyLib/Core/Numeric.h>
#include <VyLib/Core/Defines.h>
#include <VyLib/STL/Path.h>

#include <format>
namespace Vy::Utils
{
	static TString formatBytes(U64 bytes)
	{
		constexpr U64 KiB       = 1024;
		constexpr U64 MiB = KiB * 1024;
		constexpr U64 GiB = MiB * 1024;

		if (bytes >= GiB)
		{
			return std::format("{:.2f} GiB", static_cast<double>(bytes) / GiB);
		}
		if (bytes >= MiB)
		{
			return std::format("{:.2f} MiB", static_cast<double>(bytes) / MiB);
		}
		if (bytes >= KiB)
		{
			return std::format("{:.2f} KiB", static_cast<double>(bytes) / KiB);
		}

		return std::format("{} B", bytes);
	}

	// TString filenameFromPath(TPath path)
	// {
	// 	return path.stem().string();
	// }

	VY_INLINE TString filenameFromPath(TString pathString)
	{
		TPath path( pathString );

		return path.stem().string();
	}
}