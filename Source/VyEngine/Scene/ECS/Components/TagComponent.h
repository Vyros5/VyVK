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
		TString Tag{ "unnamed-entity" };

		TagComponent()                    = default;
		TagComponent(const TagComponent&) = default;
		
		TagComponent(const TString& name) : Tag(name) {}

		// Conversion operators
		operator       TString& ()       { return Tag; }
		operator const TString& () const { return Tag; }
	};
}