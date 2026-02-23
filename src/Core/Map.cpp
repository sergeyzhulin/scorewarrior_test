#include <Core/Map.h>

#include <cassert>

namespace core 
{

    void Map::create(std::uint32_t width, std::uint32_t height) 
    {
        myWidth = width;
        myHeight = height;

        const std::size_t count = static_cast<std::size_t>(myWidth) * static_cast<std::size_t>(myHeight);
        myCells.assign(count, EntityId{});
    }

    bool Map::inBounds(int x, int y) const noexcept {
        if (x < 0 || y < 0) {
            return false;
        }
        return (static_cast<std::uint32_t>(x) < myWidth) && (static_cast<std::uint32_t>(y) < myHeight);
    }

    std::size_t Map::indexOf(int x, int y) const noexcept 
    {
		assert(inBounds(x, y));
        return static_cast<std::size_t>(y) * static_cast<std::size_t>(myWidth) + static_cast<std::size_t>(x);
    }

    bool Map::isOccupied(int x, int y) const noexcept 
    {
        if (!inBounds(x, y)) {
            return false;
        }
        return !myCells[indexOf(x, y)].isNull();
    }

    EntityId Map::occupant(int x, int y) const noexcept 
    {
        if (!inBounds(x, y)) {
            return EntityId{};
        }
        return myCells[indexOf(x, y)];
    }

    bool Map::tryOccupy(int x, int y, EntityId e) noexcept 
    {
        if (e.isNull()) {
            return false;
        }
        if (!inBounds(x, y)) {
            return false;
        }

        const std::size_t idx = indexOf(x, y);
        if (!myCells[idx].isNull()) {
            return false;
        }

        myCells[idx] = e;
        return true;
    }

    bool Map::vacate(int x, int y, EntityId expected) noexcept 
    {
        if (expected.isNull()) {
            return false;
        }
        if (!inBounds(x, y)) {
            return false;
        }

        const std::size_t idx = indexOf(x, y);
        if (myCells[idx] != expected) {
            return false;
        }

        myCells[idx] = EntityId{};
        return true;
    }

    bool Map::forceVacate(int x, int y) noexcept 
    {
        if (!inBounds(x, y)) {
            return false;
        }

        myCells[indexOf(x, y)] = EntityId{};
        return true;
    }

    bool Map::move(EntityId e, int fromX, int fromY, int toX, int toY) noexcept 
    {
        if (e.isNull()) {
            return false;
        }
        if (!inBounds(fromX, fromY) || !inBounds(toX, toY)) {
            return false;
        }

        const std::size_t fromIdx = indexOf(fromX, fromY);
        const std::size_t toIdx   = indexOf(toX, toY);

        if (myCells[fromIdx] != e) {
            return false;
        }
        if (!myCells[toIdx].isNull()) {
            return false;
        }

        myCells[fromIdx] = EntityId{};
        myCells[toIdx]   = e;
        return true;
    }

    void Map::clear() noexcept 
    {
        for (auto& c : myCells) {
            c = EntityId{};
        }
    }

} // namespace core