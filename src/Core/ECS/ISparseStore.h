#pragma once
#include "EntityId.h"

namespace core 
{

    struct IEntityAlive 
    {
        virtual ~IEntityAlive() = default;
        virtual bool alive(EntityId e) const noexcept = 0;
    };

    struct ISparseStore 
    {
        virtual ~ISparseStore() = default;

        virtual void removeForAliveEntity(EntityId e) noexcept = 0;
    };

} // namespace core