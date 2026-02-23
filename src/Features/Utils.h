#pragma once
#include <Core/World.h>
#include <Core/ECS/EntityManager.h>
#include <Core/ECS/EntityId.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <vector>

namespace features 
{

	class Utils final {
	public:

		static void collectTargetsInRange(
			core::World& world,
			core::EntityId actorId,
			int minRange,
			int maxRange,
			std::vector<core::EntityId>& out) noexcept
		{
			out.clear();

			auto& em = world.entityManager();

			const auto* a = em.getComponentPtr<core::Coords>(actorId);
			if (!a) {
				return;
			}

			const auto [ax, ay] = getXY(*a);

			auto& store = em.store<core::Coords>();
			auto ents = store.denseEntities();
			auto data = store.denseData();

			double dMinRange = static_cast<double>( minRange);
			double dMaxRange = static_cast<double>(maxRange);

			for (std::size_t i = 0; i < ents.size(); ++i) {
				const core::EntityId e = ents[i];
				if (e == actorId) {
					continue;
				}

				const auto [tx, ty] = getXY(data[i]);
				const double d = MapUtils::distance(ax, ay, tx, ty);

				if (d >= dMinRange && d <= dMaxRange) {
					out.push_back(e);
				}
			}
		}

	private:

		template <class C>
		static std::pair<int, int> getXY(const C& c) noexcept
		{
			if constexpr (requires { c.x; c.y; }) {
				return { c.x, c.y };
			} else if constexpr (requires { c.value.x; c.value.y; }) {
				return { c.value.x, c.value.y };
			} else {
				return { c.coords.x, c.coords.y };
			}
		}
	};

} // namespace features