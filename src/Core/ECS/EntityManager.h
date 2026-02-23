#pragma once
#include <Core/ECS/EntityId.h>
#include <Core/ECS/ComponentTypeRegistry.h>
#include <Core/ECS/ISparseStore.h>
#include <Core/ECS/SparseComponentStore.h>

#include <cstdint>
#include <vector>
#include <memory>

namespace core {

class EntityManager final : public IEntityAlive {
public:
    explicit EntityManager(std::size_t initialCapacity = 1024,
                           std::size_t initialFreeCapacity = 1024);

    int aliveCount() const noexcept;

    EntityId createEntity();
    bool alive(EntityId id) const noexcept override;
    void destroyEntity(EntityId e) noexcept;

    // store access (lazy) Stores are createts if there is access to it
    template <class T>
	SparseComponentStore<T>& store();

    template <class T>
    T& addComponent(EntityId e) { return store<T>().add(e); }

    template <class T>
    bool removeComponent(EntityId e) noexcept { return store<T>().remove(e); }

    template <class T>
    bool tryGet(EntityId e, T& outValue) noexcept { return store<T>().tryGet(e, outValue); }

    template <class T>
    bool hasComponent(EntityId e) const noexcept { return store<T>().has(e); }

    template <class T>
    T* getComponentPtr(EntityId e) noexcept { return store<T>().getPtr(e); }


private:
    void destroyEntityIdInternal(EntityId id) noexcept;
    void ensureGenCapacity(std::uint32_t wanted);

    template <class T>
    std::unique_ptr<SparseComponentStore<T>> createStore();

private:
    std::vector<std::uint32_t> myGen;
    std::uint32_t myCount{0};

    std::vector<std::uint32_t> myFree;

    std::vector<ISparseStore*>                 myStoresByType;
    std::vector<std::unique_ptr<ISparseStore>> myStores;
};

template <class T>
inline SparseComponentStore<T>& EntityManager::store() {
    const size_t tid = ComponentType<T>::index;

    if (tid >= static_cast<int>(myStoresByType.size())) {
        myStoresByType.resize(tid + 1u, nullptr);
    }

    if (ISparseStore* existing = myStoresByType[tid]) {
        return *static_cast<SparseComponentStore<T>*>(existing);
    }

    auto created = createStore<T>();
    ISparseStore* raw = created.get();

    myStores.emplace_back(std::move(created));
    myStoresByType[tid] = raw;

    return *static_cast<SparseComponentStore<T>*>(raw);
}

template <class T>
inline std::unique_ptr<SparseComponentStore<T>> EntityManager::createStore() {
    return std::make_unique<SparseComponentStore<T>>(*this);
}

} // namespace core