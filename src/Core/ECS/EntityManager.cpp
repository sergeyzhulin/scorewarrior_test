#include <Core/ECS/EntityManager.h>

//#include <algorithm>

namespace core 
{

    EntityManager::EntityManager(std::size_t initialCapacity, std::size_t initialFreeCapacity) 
    {
        myGen.resize(initialCapacity, 0);
        myCount = 0;

        myFree.reserve(initialFreeCapacity);

        myStoresByType.reserve(128);
        myStores.reserve(128);
    }

    int EntityManager::aliveCount() const noexcept 
    {
        return static_cast<int>(myCount) - static_cast<int>(myFree.size());
    }

    EntityId EntityManager::createEntity() 
    {
        std::uint32_t idx;

        if (!myFree.empty()) {
            idx = myFree.back();
            myFree.pop_back();
        } else {
            idx = myCount;
            ensureGenCapacity(myCount + 1);
            myGen[idx] = 1;
            ++myCount;
        }

        return EntityId::make(idx, myGen[idx]);
    }

    bool EntityManager::alive(EntityId id) const noexcept 
    {
        if (id.isNull()) { 
            return false; 
        }

        const std::uint32_t idx = id.index();
        if (idx >= myCount) { 
            return false; 
        }

        return myGen[idx] == id.gen();
    }

    void EntityManager::destroyEntity(EntityId e) noexcept 
    {
        if (!alive(e)) { 
            return; 
        }

        //remove components in all created stores
        for (auto& s : myStores) {
            s->removeForAliveEntity(e);
        }

        destroyEntityIdInternal(e);
    }

    void EntityManager::destroyEntityIdInternal(EntityId id) noexcept 
    {
        const std::uint32_t idx = id.index();
        if (idx >= myCount) { 
            return; 
        }
        if (myGen[idx] != id.gen()) { 
            return; 
        }

        std::uint32_t g = myGen[idx] + 1;
        if (g == 0) {// avoid 0, generation starts with 1
            g = 1; 
        } 
        myGen[idx] = g;

        myFree.push_back(idx);
    }

    void EntityManager::ensureGenCapacity(std::uint32_t wanted) 
    {
        const std::size_t w = static_cast<std::size_t>(wanted);
        if (myGen.size() >= w) { 
            return; 
        }

        const std::size_t cap = myGen.size();
        const std::size_t newCap = std::max<std::size_t>(w, (cap == 0) ? 1024 : cap * 2);
        myGen.resize(newCap, 0);
    }

} // namespace core