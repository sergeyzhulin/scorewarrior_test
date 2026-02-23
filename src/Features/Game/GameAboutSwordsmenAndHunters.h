#pragma once
#include <fstream>
#include <iostream>
#include <IO/System/CommandParser.hpp>
#include <IO/System/EventLog.hpp>
#include <IO/System/PrintDebug.hpp>
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

#include <Core/World.h>
#include <Core/Map.h>
#include <Features/MapUtils.h>
#include <Features/Utils.h>
#include <Features/SimpleComponents.h>
#include <Features/Game/Actions.h>

using namespace sw;

namespace features {

	class GameAboutSwordsmenAndHunters final : public core::ILog
	{
	public:
		GameAboutSwordsmenAndHunters() :
				_world(this, 100)
		{}

		void play(std::ifstream& file) {

			auto noHPDeathConditionId = _world.createDeathPredicate(
				[&world = _world](core::EntityId entityId)
				{ 
					auto& em = world.entityManager();
					Health hp;
					if (em.tryGet<Health>(entityId, hp)) {
						return hp.value <= 0;
					}
					return false; 
				});

			MoveAction moveAction;
			auto moveActionId = _world.registerAction(&moveAction);

			StrikeAction heavyStrikeAction;
			auto heavyStrikeActionId = _world.registerAction(&heavyStrikeAction);

			StrikeAction shadowStrikeAction;
			auto shadowStrikeActionId = _world.registerAction(&shadowStrikeAction);

			QuickShotAction quickShotAction;
			auto quickShotActionId = _world.registerAction(&quickShotAction);

			core::Behaviour behSwordsman;
			behSwordsman.push_back( heavyStrikeActionId );
			behSwordsman.push_back(moveActionId);
			auto behSwordsmanId = _world.registerBehaviour(&behSwordsman);

			core::Behaviour behHunter;
			behHunter.push_back(quickShotActionId);
			behHunter.push_back(shadowStrikeActionId);
			behHunter.push_back(moveActionId);
			auto behHunterId = _world.registerBehaviour(&behHunter);

			std::unordered_map<uint32_t, core::EntityId> uints_map;
			std::cout << "Commands:\n";
			io::CommandParser parser;
			parser
				.add<io::CreateMap>(
					[this](auto command)
					{
						_world.map().create(command.width, command.height);
						eventLog.log(_world.worldStep(), io::MapCreated{command.width, command.height});
					})
				.add<io::SpawnSwordsman>(
					[this, &uints_map, noHPDeathConditionId, behSwordsmanId](auto command)
					{ 
						auto actorId = _world.spawnGameObject(noHPDeathConditionId, behSwordsmanId);

						auto& em = _world.entityManager();

						UnitId& uintId = em.addComponent<UnitId>(actorId);
						uintId.value = command.unitId;
						em.addComponent<Movable>(actorId);
						em.addComponent<TargetPos>(actorId);

						Health& health = em.addComponent<Health>(actorId);
						health.value = command.hp;

						Strength& strength = em.addComponent<Strength>(actorId);
						strength.value = command.strength;

						core::Coords& coords = em.addComponent<core::Coords>(actorId);
						coords.x = command.x;
						coords.y = command.y;
						_world.map().tryOccupy(coords.x, coords.y, actorId);

						uints_map.emplace(command.unitId, actorId);
						eventLog.log( _world.worldStep(),
							io::UnitSpawned{command.unitId, "Swordsman", uint32_t(coords.x), uint32_t(coords.y)});
					})
				.add<io::SpawnHunter>(
					[this, &uints_map, noHPDeathConditionId, behHunterId](auto command)
					{ 
						auto actorId = _world.spawnGameObject(noHPDeathConditionId, behHunterId);

						auto& em = _world.entityManager();

						UnitId& uintId = em.addComponent<UnitId>(actorId);
						uintId.value = command.unitId;

						em.addComponent<Movable>(actorId);
						em.addComponent<TargetPos>(actorId);

						Health& health = em.addComponent<Health>(actorId);
						health.value = command.hp;

						Strength& strength = em.addComponent<Strength>(actorId);
						strength.value = command.strength;

						core::Coords& coords = em.addComponent<core::Coords>(actorId);
						coords.x = command.x;
						coords.y = command.y;
						_world.map().tryOccupy(coords.x, coords.y, actorId);

						Agility& agility = em.addComponent<Agility>(actorId);
						agility.value = command.agility;

						Range& range = em.addComponent<Range>(actorId);
						range.minDistance = 2;
						range.maxDistance = command.range;
						uints_map.emplace(command.unitId, actorId);
						eventLog.log(
							_world.worldStep(),
							io::UnitSpawned{command.unitId, "Hunter", uint32_t(coords.x), uint32_t(coords.y)});
				})
				.add<io::March>(
					[&uints_map, this](auto command)
					{ 
						auto it = uints_map.find(command.unitId);
						assert(it != uints_map.end());
						if (it != uints_map.end()) {
							auto actorId = it->second;

							auto& em = _world.entityManager();
							core::Coords coords;
							core::Coords* pCoords = em.getComponentPtr<core::Coords>(actorId);
							if (pCoords) {
								coords = *pCoords;
							}
							TargetPos* targetPos = em.getComponentPtr<TargetPos>(actorId);
							if (targetPos) {
								targetPos->x = command.targetX;
								targetPos->y = command.targetY;
								targetPos->go = true;
							}
							eventLog.log(
								_world.worldStep(),
								io::MarchStarted{command.unitId, uint32_t(coords.x), uint32_t(coords.y), command.targetX, command.targetY});
						}
					});

			parser.parse(file);

			while (_world.alive() > 1 && _world.worldStep() < 10000) {
				_world.tick();
			}
		}

		void death(core::World& world, core::EntityId actor) override
		{
			UnitId* unitId = world.entityManager().getComponentPtr<UnitId>(actor);
			if (unitId) {
				eventLog.log(_world.worldStep(), io::UnitDied{unitId->value});
			}
		}

		void attack(core::World& world, core::EntityId actor, core::EntityId target, uint32_t damage, uint32_t targetHP) override
		{
			UnitId* unitIdActor = world.entityManager().getComponentPtr<UnitId>(actor);
			UnitId* unitIdTarget = world.entityManager().getComponentPtr<UnitId>(target);
			if (unitIdActor && unitIdTarget) {
				eventLog.log( _world.worldStep(), io::UnitAttacked{unitIdActor->value, unitIdTarget->value, damage, targetHP});
			}

		}

		void move(core::World& world, core::EntityId actor) override
		{
			UnitId* unitId = world.entityManager().getComponentPtr<UnitId>(actor);
			core::Coords* coords = world.entityManager().getComponentPtr<core::Coords>(actor);
			if (unitId && coords) {
				eventLog.log(_world.worldStep(), io::UnitMoved{unitId->value, coords->x, coords->y});
			}

		}

		void marshEnded(core::World& world, core::EntityId actor) override
		{
			UnitId* unitId = world.entityManager().getComponentPtr<UnitId>(actor);
			core::Coords* coords = world.entityManager().getComponentPtr<core::Coords>(actor);
			if (unitId && coords) {
				eventLog.log(_world.worldStep(), io::MarchEnded{unitId->value, coords->x, coords->y});
			}

		}

	private:

	private:
		core::World _world;
		sw::EventLog eventLog;
	};

} // namespace features