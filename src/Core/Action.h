#pragma once
#include <Core/ECS/EntityId.h>
#include <Core/ActionId.h>

namespace core 
{

	class World;

	struct Action 
	{
		virtual ~Action() = default;
		virtual bool tryExecute(World& world, EntityId actorId, ActionId actionId) noexcept = 0;
	};

} // namespace core