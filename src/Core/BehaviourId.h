#pragma once
#include <cstddef>

namespace core 
{

    struct BehaviourId final 
    {
        std::size_t myValue{0}; // 0 == null

        constexpr BehaviourId() = default;
        explicit constexpr BehaviourId(std::size_t v) : myValue(v) {}

        constexpr bool isNull() const noexcept { return myValue == 0; }
        constexpr std::size_t value() const noexcept { return myValue; }

        friend constexpr bool operator==(BehaviourId a, BehaviourId b) noexcept { return a.myValue == b.myValue; }
        friend constexpr bool operator!=(BehaviourId a, BehaviourId b) noexcept { return a.myValue != b.myValue; }
    };

} // namespace core