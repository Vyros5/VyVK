#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
    /**
     * @brief Gives a name to an Entity.
     * 
     * @note Required Component
     */
	struct TagComponent
	{
		String Tag{"Entity"};

		TagComponent()                    = default;
		TagComponent(const TagComponent&) = default;
		
		TagComponent(const String& name) : Tag(name) {}

		// Conversion operators
		operator       String& ()       { return Tag; }
		operator const String& () const { return Tag; }
	};
}