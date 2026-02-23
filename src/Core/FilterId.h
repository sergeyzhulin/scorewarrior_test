#pragma once
#include <cstddef>

namespace core 
{

    struct FilterId final 
    {
        std::size_t myValue{0}; // 0 == null - stub

        constexpr FilterId() = default;
        explicit constexpr FilterId(std::size_t v) : myValue(v) {}

        constexpr bool isNull() const noexcept { return myValue == 0; }
        constexpr std::size_t value() const noexcept { return myValue; }

        friend constexpr bool operator==(FilterId a, FilterId b) noexcept { return a.myValue == b.myValue; }
        friend constexpr bool operator!=(FilterId a, FilterId b) noexcept { return a.myValue != b.myValue; }
    };

} // namespace core