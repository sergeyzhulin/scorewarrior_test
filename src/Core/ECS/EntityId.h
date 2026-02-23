#pragma once
#include <cstdint>

namespace core 
{

    struct EntityId final 
    {
    public:
        static constexpr std::uint64_t low32Mask = 0xFFFF'FFFFull;

        constexpr EntityId() = default;
        explicit constexpr EntityId(std::uint64_t raw) : myValue(raw) {}

        static constexpr EntityId make(std::uint32_t index, std::uint32_t gen) noexcept 
        {
            const std::uint64_t raw = (std::uint64_t(gen) << 32) | std::uint64_t(index);
            return EntityId(raw);
        }

        constexpr std::uint32_t index() const noexcept 
        {
            return static_cast<std::uint32_t>(myValue & low32Mask);
        }

        constexpr std::uint32_t gen() const noexcept 
        {
            return static_cast<std::uint32_t>((myValue >> 32) & low32Mask);
        }

        constexpr std::uint64_t raw() const noexcept { return myValue; }
        constexpr bool isNull() const noexcept { return myValue == 0; }

        friend constexpr bool operator==(EntityId a, EntityId b) noexcept 
        {
            return a.myValue == b.myValue;
        }
        friend constexpr bool operator!=(EntityId a, EntityId b) noexcept 
        {
            return a.myValue != b.myValue;
        }

	private:
		std::uint64_t myValue{0};
	};

} // namespace core