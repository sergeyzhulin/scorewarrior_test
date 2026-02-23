#pragma once
#include <cstddef>

namespace core 
{

	struct ActionId final 
	{
		std::size_t myValue{0}; // 0 == null - it is used as stub

		constexpr ActionId() = default;
		explicit constexpr ActionId(std::size_t v) : myValue(v) {}

		constexpr bool isNull() const noexcept { return myValue == 0; }
		constexpr std::size_t value() const noexcept { return myValue; }

		friend constexpr bool operator==(ActionId a, ActionId b) noexcept { return a.myValue == b.myValue; }
		friend constexpr bool operator!=(ActionId a, ActionId b) noexcept { return a.myValue != b.myValue; }
	};

} // namespace core