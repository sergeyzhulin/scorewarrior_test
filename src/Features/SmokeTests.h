#include "IO/System/TypeRegistry.hpp"

#include <IO/Commands/CreateMap.hpp>
#include <IO/Commands/March.hpp>
#include <IO/Commands/SpawnHunter.hpp>
#include <IO/Commands/SpawnSwordsman.hpp>
#include <IO/Events/MapCreated.hpp>
#include <IO/Events/MarchEnded.hpp>
#include <IO/Events/MarchStarted.hpp>
#include <IO/Events/UnitAttacked.hpp>
#include <IO/Events/UnitDied.hpp>
#include <IO/Events/UnitMoved.hpp>
#include <IO/Events/UnitSpawned.hpp>
#include <IO/System/CommandParser.hpp>
#include <IO/System/EventLog.hpp>
#include <IO/System/PrintDebug.hpp>
#include <fstream>
#include <iostream>
#include <Core/World.h>

namespace tests
{

	struct Health
	{
		int value{0};
	};

	class SmokeTests final
	{
	public:
		static void simpleTestECS() {
			core::EntityManager em;
			core::EntityId npc = em.createEntity();
			core::EntityId npc2 = em.createEntity();
			Health& npcHealth = em.addComponent<Health>(npc);
			Health& npcHealth2 = em.addComponent<Health>(npc2);
			npcHealth.value = 100;
			npcHealth2.value = 50;

			std::span<Health> healths = em.store<Health>().denseData();
			Health tmp;
			em.tryGet<Health>(npc, tmp);
			em.tryGet<Health>(npc2, tmp);
		}

		static void simpleTestForBehsAndActions() {
				class TraceAction : public core::Action
				{
					bool tryExecute(core::World&, core::EntityId actorId, core::ActionId) noexcept override
					{
						std::cout << "TraceAction completed by unit: " << actorId.index() << std::endl;
						return true;
					}
				};

				core::World world(nullptr, 100);

				TraceAction traceAction;
				const auto traceActionId = world.registerAction(&traceAction);

				core::Behaviour beh;
				beh.push_back(traceActionId);

				const auto behId = world.registerBehaviour(&beh);

				auto deathId = world.createDeathPredicate([](core::EntityId) { return true; });

				auto npc0 = world.spawnGameObject({}, behId);

				world.tick();
				world.tick();
				world.tick();
		}

		static void testForActionsAndFilters() {
			core::World world(nullptr, 100);
			auto id = world.spawnGameObject({}, {});

			static core::EntityId ravenId;

			struct MeleeAttackParams final
			{
				int damage{0};
			};

			struct RangedAttackParams final
			{
				int minRange{2};
				int maxRange{2};
				int damage{0};
			};

			struct RavenFilter final
			{
				int rangePenalty{1};

				bool apply(
					core::World&, core::EntityId, core::EntityId, core::ActionId, MeleeAttackParams&) const noexcept {
					return false;  // deny melee
				}

				bool apply(
					core::World&, core::EntityId, core::EntityId, core::ActionId, RangedAttackParams& p) const noexcept {
					p.minRange = std::max(1, p.minRange - rangePenalty);
					p.maxRange = std::max(1, p.maxRange - rangePenalty);

					if (p.maxRange < p.minRange)
					{
						p.maxRange = p.minRange;
					}
					return true;
				}
			};

			class MoveAction : public core::Action
			{
				bool tryExecute(core::World&, core::EntityId actorId, core::ActionId) noexcept override
				{
					std::cout << "cannot Move unit: " << actorId.index() << std::endl;
					return false;
				}
			};

			static core::ActionId complexActionId;

			class ComplexAttackAction : public core::Action
			{
				bool tryExecute(core::World& world, core::EntityId actorId, core::ActionId) noexcept override
				{
					RangedAttackParams params;
					params.damage = 10;
					params.minRange = 2;
					params.maxRange = 5;

					core::ActionFilters* af = world.tryGetActionFilters(ravenId);
					auto fid = af->get(complexActionId);

					const bool allowed = world.applyFilter(fid, actorId, actorId, complexActionId, params);
					if (!allowed) {
						return false;
					}

					std::cout << "ComplexAttackAction completed by unit: " << actorId.index() << std::endl;
					return true;
				}
			};

			MoveAction moveAction;
			const auto moveActionId = world.registerAction(&moveAction);

			ComplexAttackAction complexAction;
			complexActionId = world.registerAction(&complexAction);

			core::Behaviour beh;
			beh.push_back(moveActionId);
			beh.push_back(complexActionId);

			auto behId = world.registerBehaviour(&beh);

			ravenId = world.spawnGameObject({}, behId);

			RavenFilter ravenFilter;

			core::FilterId ravenMeleeFilterId = world.registerFilter<RavenFilter, MeleeAttackParams>(&ravenFilter);
			core::FilterId ravenRangeFilterId = world.registerFilter<RavenFilter, RangedAttackParams>(&ravenFilter);

			auto& ravenFilters = world.ensureActionFilters(ravenId);
			ravenFilters.set(moveActionId, ravenMeleeFilterId);
			ravenFilters.set(complexActionId, ravenRangeFilterId);

			auto& rf = world.ensureActionFilters(ravenId);
			(void)rf;

			world.tick();
			world.tick();
			world.tick();
		}


	};

}
