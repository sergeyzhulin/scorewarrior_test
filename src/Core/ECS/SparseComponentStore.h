#pragma once
#include "ISparseStore.h"

#include <vector>
#include <span>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>

namespace core 
{

    template <class T>
    class SparseComponentStore final : public ISparseStore {
    public:
        static constexpr std::int32_t none = -1;

        explicit SparseComponentStore(IEntityAlive& aliveProvider,
                                      std::size_t initialEntityCapacity = 1024,
                                      std::size_t initialDenseCapacity  = 256)
            : myAlive(aliveProvider)
        {
            mySparse.resize(initialEntityCapacity, none);
            myDenseEntities.reserve(std::max<std::size_t>(1, initialDenseCapacity));
            myDenseData.reserve(std::max<std::size_t>(1, initialDenseCapacity));
        }

        int count() const noexcept 
        { 
            return static_cast<int>(myDenseData.size()); 
        }

        std::span<const EntityId> denseEntities() const noexcept 
        {
            return { myDenseEntities.data(), myDenseEntities.size() };
        }

        std::span<T> denseData() noexcept 
        {
            return { myDenseData.data(), myDenseData.size() };
        }

        std::span<const T> denseData() const noexcept 
        {
            return { myDenseData.data(), myDenseData.size() };
        }

        bool has(EntityId e) const noexcept 
        {
            if (!alive(e)) { 
                return false; 
            }

            const std::uint32_t idx = e.index();
            if (idx >= mySparse.size()) { 
                return false; 
            }

            const std::int32_t di = mySparse[idx];
            if (di < 0) { 
                return false; 
            }

            const auto udi = static_cast<std::size_t>(di);
            if (udi >= myDenseEntities.size()) { 
                return false; 
            }

            return myDenseEntities[udi] == e;
        }

        bool tryGet(EntityId e, T& outValue) const noexcept 
        {
            if (!alive(e)) { 
                outValue = T{}; 
                return false; 
            }

            const std::uint32_t idx = e.index();
            if (idx >= mySparse.size()) { 
                outValue = T{}; 
                return false; 
            }

            const std::int32_t di = mySparse[idx];
            if (di < 0) { 
                outValue = T{}; 
                return false; 
            }

            const auto udi = static_cast<std::size_t>(di);
            if (udi >= myDenseEntities.size()) { 
                outValue = T{}; 
                return false; 
            }

            if (myDenseEntities[udi] != e) { 
                outValue = T{}; 
                return false; 
            }

            outValue = myDenseData[udi];
            return true;
        }

        T* getPtr(EntityId e) noexcept 
        {
            if (!alive(e)) { 
                return nullptr; 
            }

            const std::uint32_t idx = e.index();
            if (idx >= mySparse.size()) { 
                return nullptr; 
            }

            const std::int32_t di = mySparse[idx];
            if (di < 0) { 
                return nullptr; 
            }

            const auto udi = static_cast<std::size_t>(di);
            if (udi >= myDenseEntities.size()) { 
                return nullptr; 
            }

            if (myDenseEntities[udi] != e) { 
                return nullptr; 
            }

            return &myDenseData[udi];
        }

        const T* getPtr(EntityId e) const noexcept 
        {
            return const_cast<SparseComponentStore*>(this)->getPtr(e);
        }

        T& add(EntityId e) 
        {
            assert(alive(e) && "cannot add component to a dead entity");

            const std::uint32_t idx = e.index();
            ensureSparseCapacity(idx);

            std::int32_t di = mySparse[idx];
            if (di >= 0) {
                const auto udi = static_cast<std::size_t>(di);
                if (udi < myDenseEntities.size()) {
                    const EntityId existing = myDenseEntities[udi];

                    if (existing == e) {
                        return myDenseData[udi];
                    }

                    if (!existing.isNull() && existing.index() == idx) {
                        // stale mapping due to index reuse
                        removeDenseAt(udi);
                        di = none;
                    }
                }
            }

            ensureDenseCapacity(myDenseData.size() + 1);

            myDenseEntities.push_back(e);
            myDenseData.emplace_back(T{});
            const std::size_t newDi = myDenseData.size() - 1;

            mySparse[idx] = static_cast<std::int32_t>(newDi);
            return myDenseData[newDi];
        }

        bool remove(EntityId e) noexcept 
        {
            if (!alive(e)) { 
                return false; 
            }

            const std::uint32_t idx = e.index();
            if (idx >= mySparse.size()) { 
                return false; 
            }

            const std::int32_t di = mySparse[idx];
            if (di < 0) { 
                return false; 
            }

            const auto udi = static_cast<std::size_t>(di);
            if (udi >= myDenseEntities.size()) { 
                return false; 
            }

            if (myDenseEntities[udi] != e) { 
                return false; 
            }

            removeDenseAt(udi);
            return true;
        }

        void removeForAliveEntity(EntityId e) noexcept override 
        {
            const std::uint32_t idx = e.index();
            if (idx >= mySparse.size()) { 
                return; 
            }

            const std::int32_t di = mySparse[idx];
            if (di < 0) { 
                return; 
            }

            const auto udi = static_cast<std::size_t>(di);
            if (udi >= myDenseEntities.size()) { 
                return; 
            }

            if (myDenseEntities[udi] != e) { 
                return; 
            }

            removeDenseAt(udi);
        }

    private:
        bool alive(EntityId e) const noexcept 
        { 
            return myAlive.alive(e); 
        }

        void removeDenseAt(std::size_t di) noexcept 
        {
            const std::size_t last = myDenseData.size() - 1;
            const EntityId removedEntity = myDenseEntities[di];

            if (di != last) {
                const EntityId movedEntity = myDenseEntities[last];

                myDenseEntities[di] = movedEntity;
                myDenseData[di]     = std::move(myDenseData[last]);

                const std::uint32_t movedIdx = movedEntity.index();
                if (movedIdx < mySparse.size()) {
                    mySparse[movedIdx] = static_cast<std::int32_t>(di);
                }
            }

            myDenseEntities.pop_back();
            myDenseData.pop_back();

            const std::uint32_t removedIdx = removedEntity.index();
            if (removedIdx < mySparse.size()) {
                mySparse[removedIdx] = none;
            }
        }

        void ensureSparseCapacity(std::uint32_t entityIndex) 
        {
            const std::size_t wanted = static_cast<std::size_t>(entityIndex) + 1;
            if (mySparse.size() >= wanted) { 
                return; 
            }

            const std::size_t oldLen = mySparse.size();
            const std::size_t newLen = std::max<std::size_t>(wanted, (oldLen == 0) ? 1024 : oldLen * 2);
            mySparse.resize(newLen, none);
        }

        void ensureDenseCapacity(std::size_t wanted) 
        {
            if (myDenseData.capacity() >= wanted) { 
                return; 
            }

            const std::size_t cap = myDenseData.capacity();
            const std::size_t newCap = std::max<std::size_t>(wanted, (cap == 0) ? 256 : cap * 2);

            myDenseData.reserve(newCap);
            myDenseEntities.reserve(newCap);
        }

    private:
        IEntityAlive& myAlive;
        std::vector<std::int32_t> mySparse;        // entityIndex -> denseIndex, -1 if none
        std::vector<EntityId>     myDenseEntities; // denseIndex -> EntityId
        std::vector<T>            myDenseData;     // denseIndex -> component
    };

} // namespace core