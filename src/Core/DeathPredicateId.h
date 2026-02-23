#pragma once
#include <cstddef>

namespace core 
{

    struct DeathPredicateId final 
    {
        std::size_t myValue{0}; // 0 == null it is stub

        constexpr DeathPredicateId() = default;
        explicit constexpr DeathPredicateId(std::size_t v) : myValue(v) {}

        constexpr bool isNull() const noexcept { return myValue == 0; }
        constexpr std::size_t value() const noexcept { return myValue; }

        friend constexpr bool operator==(DeathPredicateId a, DeathPredicateId b) noexcept { return a.myValue == b.myValue; }
        friend constexpr bool operator!=(DeathPredicateId a, DeathPredicateId b) noexcept { return a.myValue != b.myValue; }
    };

} // namespace core