#pragma once

namespace features {

	struct MeleeAttackParams final
	{
		uint32_t damage{0};
	};

	struct RangedAttackParams final
	{
		uint32_t minRange{2};
		uint32_t maxRange{2};
		uint32_t damage{0};
	};

	// Move to targetPos if there is go==true (it is marsh), when thereis no marsh, move just to near target
	class MoveAction : public core::Action {
	public:

		bool tryExecute(core::World& world, core::EntityId actorId, core::ActionId) noexcept override
		{
			auto& em = world.entityManager();
			core::Coords* coords = em.getComponentPtr<core::Coords>(actorId);
			if (!coords) {
				return false;
			}
			TargetPos* targetPos = em.getComponentPtr<TargetPos>(actorId);
			if (targetPos && targetPos->go) {
				if (targetPos->x != coords->x || targetPos->y != coords->y) {
					return moveTo(world, actorId, targetPos->x, targetPos->y);
				}
				targetPos->go = false; //Marsh finished
				
				if (world.logger()) {
					world.logger()->marshEnded(world, actorId);
				}
			}

			//now, move just to near target
			auto target = MapUtils::findNearestOccupied(world.map(), coords->x, coords->y);
			if (!target.isValid()) {
				return false;
			}

			return moveTo(world, actorId, target.x, target.y);
		}

		bool moveTo(core::World& world, core::EntityId actorId, int x, int y)
		{
			auto& em = world.entityManager();

			core::Coords* coords = em.getComponentPtr<core::Coords>(actorId);
			if (!coords) {
				return false;
			}

			const int cx = coords->x;
			const int cy = coords->y;

			if (cx == x && cy == y)
			{
				return false;
			}

			const int dx = (x > cx) ? 1 : (x < cx ? -1 : 0);
			const int dy = (y > cy) ? 1 : (y < cy ? -1 : 0);

			const int nx = cx + dx;
			const int ny = cy + dy;

			auto& map = world.map();

			if (!map.inBounds(nx, ny)) {
				return false;
			}
			if (map.isOccupied(nx, ny)) {
				return false;
			}

			if (!map.move(actorId, cx, cy, nx, ny)) {
				return false;
			}

			coords->x = nx;
			coords->y = ny;
			if (world.logger()) {
				world.logger()->move( world, actorId );
			}

			return true;
		}
	};

	class StrikeAction : public core::Action
	{
		bool tryExecute(core::World& world, core::EntityId actorId, core::ActionId actionId) noexcept override
		{

			auto& em = world.entityManager();
			core::Coords* coords = em.getComponentPtr<core::Coords>(actorId);
			if (!coords) {
				return false;
			}

			Strength* strength = em.getComponentPtr<Strength>(actorId);
			if (!strength || strength->value == 0) {
				return false;
			}

			std::vector<core::EntityId> neigbours;
			neigbours.reserve(8);
			MapUtils::collectAdjacentOccupied(world.map(), coords->x, coords->y, neigbours, actorId);

			if (neigbours.empty()) {
				return false;
			}

			auto targetId = neigbours[world.random(0, static_cast<uint32_t>(neigbours.size() - 1))];
			Health* targetHealth = em.getComponentPtr<Health>(targetId);
			if (!targetHealth) {
				return false;
			}

			MeleeAttackParams params;
			params.damage = strength->value;

			//There is processing, if it is possible to strike target. And target can correct inflicted damage/
			core::ActionFilters* af = world.tryGetActionFilters(targetId);
			if (af) {
				auto fid = af->get(actionId);
				if (!fid.isNull()) {
					const bool allowed = world.applyFilter(fid, actorId, targetId, actionId, params);
					if (!allowed) {
						return false;
					}
				}
			}

			targetHealth->value -= std::min(targetHealth->value, params.damage);

			if (world.logger()) {
				world.logger()->attack(world, actorId, targetId, params.damage, targetHealth->value);
			}

			return true;
		}

	};

	class QuickShotAction : public core::Action
	{
		bool tryExecute(core::World& world, core::EntityId actorId, core::ActionId actionId) noexcept override
		{
			auto& em = world.entityManager();
			core::Coords* coords = em.getComponentPtr<core::Coords>(actorId);
			if (!coords) {
				return false;
			}

			std::vector<core::EntityId> neigbours;
			neigbours.reserve(8);
			MapUtils::collectAdjacentOccupied(world.map(), coords->x, coords->y, neigbours, actorId);
			if (!neigbours.empty()) {
				return false; //cannot shot if somebody is near shoter
			}

			Agility* agility = em.getComponentPtr<Agility>(actorId);
			if (!agility || agility->value == 0) {
				return false;
			}

			Range* range = em.getComponentPtr<Range>(actorId);
			if (!range) {
				return false;
			}

			std::vector<core::EntityId> targets;
			targets.reserve(8);
			Utils::collectTargetsInRange(world, actorId, range->minDistance, range->maxDistance, targets);

			if (targets.empty()) {
				return false;
			}

			auto targetId = targets[world.random(0, static_cast<uint32_t>(targets.size() - 1))];
			Health* targetHealth = em.getComponentPtr<Health>(targetId);
			if (!targetHealth) {
				return false;
			}

			RangedAttackParams params;
			params.damage = agility->value;
			params.minRange = range->minDistance;
			params.maxRange = range->maxDistance;

			core::ActionFilters* af = world.tryGetActionFilters(targetId);
			if (af) {
				auto fid = af->get(actionId);
				if (!fid.isNull()) {
					const bool allowed = world.applyFilter(fid, actorId, targetId, actionId, params);
					if (!allowed) {
						return false;
					}
				}
			}

			targetHealth->value -= std::min(targetHealth->value, params.damage);

			if (world.logger()) {
				world.logger()->attack(world, actorId, targetId, params.damage, targetHealth->value);
			}

			return true;
		}
	};

} // namespace features