#pragma once
#include "DeathPredicateId.h"

namespace core 
{

    // semantics:
    // - component absent, immortal object
    // - predicateId == 0. terminator and ready to destroy during cleanup process
    struct DeathCondition final 
    {
        DeathPredicateId predicateId{};
    };

} // namespace core