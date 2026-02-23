#pragma once
#include <Core/ActionFilters.h>

namespace core 
{

	// ActionId -> FilterId
	struct ActionFiltersComponent final {
		ActionFilters filters;
	};

} // namespace core