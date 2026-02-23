#pragma once
#include <Core/ECS/EntityManager.h>
#include <Core/ECS/EntityId.h>
#include <Core/DeathPredicateId.h>
#include <Core/DeathCondition.h>

#include <Core/Action.h>
#include <Core/ActionId.h>
#include <Core/Behaviour.h>
#include <Core/BehaviourId.h>
#include <Core/BehaviourComponent.h>

#include <Core/FilterId.h>
#include <Core/ActionFiltersComponent.h>
#include <Core/Map.h>

#include <functional>
#include <vector>
#include <cstddef>
#include <cassert>
#include <concepts>
#include <random>

namespace core 
{

    class World;

    class ILog
    {
    public:
	    virtual ~ILog() = default;
	    virtual void death(World& world, EntityId actor) = 0;
	    virtual void attack(World& world, EntityId actor, EntityId target, uint32_t damage, uint32_t targetHP) = 0;
	    virtual void move(World& world, EntityId actor) = 0;
	    virtual void marshEnded(World& world, EntityId actor) = 0;
    };

    struct Coords
    {
	    uint32_t x{0};
	    uint32_t y{0};
    };	


    class World final 
    {
    public:
	    std::mt19937 rng{std::random_device{}()};

        using deathPredicate = std::function<bool(EntityId)>;

        explicit World(ILog* logger = nullptr, std::size_t gameObjectReserve = 0);

        DeathPredicateId createDeathPredicate(deathPredicate pred);
        ActionId registerAction(Action* action) noexcept;
        BehaviourId registerBehaviour(Behaviour* behaviour) noexcept;

        template <class TFilter, class TParams>
        FilterId registerFilter(TFilter* filter) noexcept;

        template <class TParams>
        bool applyFilter(FilterId filterId, 
                         EntityId actorId,
                         EntityId targetId,
                         ActionId actionId,
                         TParams& inOut) noexcept;

        // deathPid:
        // - 0 => immortal object and no DeathCondition component will be created
        // - >0 => DeathCondition created with this predicate
        // behaviourId:
        // - 0 => no behaviour component
        EntityId spawnGameObject(DeathPredicateId deathPid = DeathPredicateId{}, BehaviourId behaviourId = BehaviourId{});

        //helpers for Action filters
        ActionFilters& ensureActionFilters(EntityId targetId) noexcept;
        ActionFilters* tryGetActionFilters(EntityId targetId) noexcept;

        void tick();

        uint32_t random(uint32_t min, uint32_t max) 
        {
		    return std::uniform_int_distribution<int>(min, max)(rng);
        }

        EntityManager& entityManager() noexcept 
        { 
            return myEntityManager; 
        }

        Map& map() noexcept 
        {
            return myMap; 
        }
        ILog* logger() noexcept 
        {
 
            return myLogger; 
        }
        const EntityManager& entityManager() const noexcept 
        { 
            return myEntityManager; 
        }

        size_t worldStep() const 
        {
		    return myWorldStep;
        }

        int alive() const noexcept
        {
			std::size_t v = 0;

			if (myGameObjectCount > myGameObjectsRipCount) {
				v = myGameObjectCount - myGameObjectsRipCount;
			}

			return static_cast<int>(v);
        }

    private:
        void compactGameObjectsIfNeeded() noexcept;

        bool isTerminator(EntityId e) noexcept;

        void executeActions() noexcept;

    private:
        EntityManager myEntityManager;
	    Map myMap;
	    ILog* myLogger = nullptr;

        std::vector<deathPredicate> myDeathPredicates;

        // id==0 reserved (null), lifetime owned by caller
        std::vector<Action*>   myActions;
        std::vector<Behaviour*> myBehaviours;

        // filters registry (type-erasure), id==0 reserved (null), lifetime owned by caller
        struct FilterEntry final 
        {
            void* myObject{nullptr};

            bool (*myApplyFn)(void* object,
                              World& world,
                              EntityId actorId,
                              EntityId targetId,
                              ActionId actionId,
                              void* params) noexcept { nullptr };
        };
        std::vector<FilterEntry> myFilters;

        // spawn order (no shrink), used length tracked separately
        std::vector<EntityId> myGameObjects;
        std::size_t myGameObjectCount{0};
        std::size_t myGameObjectsRipCount{0};
	    size_t myWorldStep = 1;
    };

    template <class TFilter, class TParams>
    inline FilterId World::registerFilter(TFilter* filter) noexcept
    {
        assert(filter != nullptr);

        static_assert(
            requires(TFilter& f, World& w, EntityId a, EntityId t, ActionId act, TParams& p) {
                { f.apply(w, a, t, act, p) } noexcept -> std::same_as<bool>;
            },
            "Filter must provide: bool apply(World&, EntityId, EntityId, ActionId, TParams&) noexcept"
        );

        auto thunk = +[](void* obj, World& w, EntityId a, EntityId t, ActionId act, void* params) noexcept -> bool {
            auto& f = *static_cast<TFilter*>(obj);
            auto& p = *static_cast<TParams*>(params);
            return f.apply(w, a, t, act, p);
        };

        const std::size_t idx = myFilters.size();
        myFilters.push_back(FilterEntry{ filter, thunk });

        return FilterId{ idx }; // idx >= 1 (0 reserved)
    }

    template <class TParams>
    inline bool World::applyFilter(FilterId filterId,
                                   EntityId actorId,
                                   EntityId targetId,
                                   ActionId actionId,
                                   TParams& inOut) noexcept
    {
        if (filterId.isNull()) {
            return true; // no filter, action allowed
        }

        const std::size_t idx = filterId.value();
        assert(idx < myFilters.size());

        FilterEntry& e = myFilters[idx];
        assert(e.myObject != nullptr);
        assert(e.myApplyFn != nullptr);

        return e.myApplyFn(e.myObject, *this, actorId, targetId, actionId, &inOut);
    }

} // namespace core