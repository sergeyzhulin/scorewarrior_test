#include <Core/World.h>

#include <cassert>

namespace core 
{

    World::World(ILog* logger, std::size_t gameObjectReserve)
        : myEntityManager()
    {
	    myLogger = logger;

        if (gameObjectReserve > 0) {
            myGameObjects.reserve(gameObjectReserve);
        }

        // reserve id=0 for "null"
        myDeathPredicates.emplace_back([](EntityId) { return false; });

        // reserve id=0 for "null"
        myActions.push_back(nullptr);
        myBehaviours.push_back(nullptr);

        // reserve id=0 for "null"
        myFilters.push_back(FilterEntry{});

        // ensure internal stores exist
        (void)myEntityManager.store<DeathCondition>();
        (void)myEntityManager.store<BehaviourComponent>();
        (void)myEntityManager.store<ActionFiltersComponent>();
    }

    DeathPredicateId World::createDeathPredicate(deathPredicate pred) 
    {
        assert(static_cast<bool>(pred) && "death predicate must be callable");

        const std::size_t idx = myDeathPredicates.size();
        myDeathPredicates.emplace_back(std::move(pred));
        return DeathPredicateId{ idx }; // idx >= 1
    }

    ActionId World::registerAction(Action* action) noexcept 
    {
        assert(action != nullptr);

        const std::size_t idx = myActions.size();
        myActions.push_back(action);
        return ActionId{ idx }; // idx >= 1
    }

    BehaviourId World::registerBehaviour(Behaviour* behaviour) noexcept 
    {
        assert(behaviour != nullptr);

        const std::size_t idx = myBehaviours.size();
        myBehaviours.push_back(behaviour);
        return BehaviourId{ idx }; // idx >= 1
    }

    EntityId World::spawnGameObject(DeathPredicateId deathPid, BehaviourId behaviourId) 
    {
        const EntityId e = myEntityManager.createEntity();

        if (myGameObjectCount < myGameObjects.size()) {
            myGameObjects[myGameObjectCount] = e;
        } else {
            myGameObjects.push_back(e);
        }
        ++myGameObjectCount;

        // DeathCondition is hidden implementation detail:
        // no component, object is immortal
        // component with predicateId==0 => terminator, destroy when cleanup
        if (!deathPid.isNull()) {
            auto& dc = myEntityManager.addComponent<DeathCondition>(e);
            dc.predicateId = deathPid;
        }

        if (!behaviourId.isNull()) {
            auto& bc = myEntityManager.addComponent<BehaviourComponent>(e);
            bc.behaviourId = behaviourId;
        }

        return e;
    }

    ActionFilters& World::ensureActionFilters(EntityId targetId) noexcept 
    {
        assert(myEntityManager.alive(targetId));

        if (auto* c = myEntityManager.getComponentPtr<ActionFiltersComponent>(targetId)) {
            return c->filters;
        }

        auto& created = myEntityManager.addComponent<ActionFiltersComponent>(targetId);
        return created.filters;
    }

    ActionFilters* World::tryGetActionFilters(EntityId targetId) noexcept 
    {
        if (!myEntityManager.alive(targetId)) {
            return nullptr;
        }
        if (auto* c = myEntityManager.getComponentPtr<ActionFiltersComponent>(targetId)) {
            return &c->filters;
        }
        return nullptr;
    }

    bool World::isTerminator(EntityId e) noexcept 
    {
        // terminator: has DeathCondition && predicateId == 0
        if (auto* dc = myEntityManager.getComponentPtr<DeathCondition>(e)) {
            return dc->predicateId.isNull();
        }
        return false;
    }

    void World::tick() 
    {
	    ++myWorldStep;
        // cleanup phase terminators
        //  Terminator semantics: has DeathCondition && predicateId==0 => destroyEntity now and remove from spawn list
        for (std::size_t i = 0; i < myGameObjectCount; ++i) {
            EntityId& slot = myGameObjects[i];
            if (slot.isNull()) {
                continue;
            }

            const EntityId e = slot;

            if (!myEntityManager.alive(e)) {
                slot = EntityId{};
				++myGameObjectsRipCount;
                continue;
            }

            if (isTerminator(e)) {
			    auto* coords = myEntityManager.getComponentPtr<Coords>(e);
			    if (coords) {
				    myMap.forceVacate(coords->x, coords->y);
			    }
			    if (myLogger) {
				    myLogger->death(*this, e);
			    }

                myEntityManager.destroyEntity(e);
                slot = EntityId{};
				++myGameObjectsRipCount;
                continue;
            }
        }

        //actions phase
	    executeActions();

        // phase: evaluate death predicates and set terminator (predicateId=0)
        for (std::size_t i = 0; i < myGameObjectCount; ++i) {
            const EntityId e = myGameObjects[i];
            if (e.isNull()) {
                continue;
            }
            if (!myEntityManager.alive(e)) {
                continue;
            }

            auto* dc = myEntityManager.getComponentPtr<DeathCondition>(e);
            if (!dc) {
                continue; // immortal objects (no DeathCondition component)
            }

            const DeathPredicateId pid = dc->predicateId;
            if (pid.isNull()) {
                continue; // already terminator amd will be destroyed next tick
            }

            const std::size_t idx = pid.value();
            assert(idx < myDeathPredicates.size());

            if (myDeathPredicates[idx](e)) {
                dc->predicateId = DeathPredicateId{}; // set terminator
            }
        }

        //compact phase, pack spawn order list
        compactGameObjectsIfNeeded();
    }

    void World::executeActions() noexcept
    {
	    for (std::size_t i = 0; i < myGameObjectCount; ++i) {
		    const EntityId e = myGameObjects[i];
		    if (e.isNull()) {
			    continue;
		    }
		    if (!myEntityManager.alive(e)) {
			    continue;
		    }
		    if (isTerminator(e)) {
			    continue;
		    }

		    const auto* bc = myEntityManager.getComponentPtr<BehaviourComponent>(e);
		    if (!bc) {
			    continue;
		    }

		    const BehaviourId bid = bc->behaviourId;
		    if (bid.isNull()) {
			    continue;
		    }

		    const std::size_t bidx = bid.value();
		    assert(bidx < myBehaviours.size());

		    Behaviour* behaviour = myBehaviours[bidx];
		    if (!behaviour) {
			    continue;
		    }

		    for (const ActionId aid : behaviour->actions()) {
			    if (aid.isNull()) {
				    continue;
			    }

			    const std::size_t aidx = aid.value();
			    assert(aidx < myActions.size());

			    Action* action = myActions[aidx];
			    if (!action) {
				    continue;
			    }

			    if (action->tryExecute(*this, e, aid)) {
				    break;	// one action per tick for this entity
			    }
		    }
	    }
    }

    void World::compactGameObjectsIfNeeded() noexcept 
    {
		if (myGameObjectCount == 0 || myGameObjectsRipCount == 0) {
            return;
        }
        // 33% gap, time to compact
		if (myGameObjectsRipCount * 3 < myGameObjectCount) {
            return;
        }

        std::size_t w = 0;
        for (std::size_t r = 0; r < myGameObjectCount; ++r) {
            const EntityId e = myGameObjects[r];
            if (!e.isNull()) {
                myGameObjects[w++] = e;
            }
        }

        myGameObjectCount = w;
		myGameObjectsRipCount = 0;
    }

} // namespace core