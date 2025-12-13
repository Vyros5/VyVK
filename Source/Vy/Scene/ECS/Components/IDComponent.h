#pragma once

#include <VyLib/Common/VyUUID.h>

namespace Vy
{
    /**
     * @brief Gives a VyUUID value to an Entity.
     * 
     * @note Required Component
     */
	struct IDComponent
	{
		VyUUID ID;

		IDComponent()                   = default;
		IDComponent(const IDComponent&) = default;
		
		IDComponent(const VyUUID& uuid) : ID(uuid) {}

		// Conversion operators
		operator       VyUUID& ()       { return ID; }
		operator const VyUUID& () const { return ID; }
	};

}