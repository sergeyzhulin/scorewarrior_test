#pragma once
#include <Core/ECS/EntityId.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace core 
{

    class Map final 
    {
    public:
        Map() = default;

        Map(std::uint32_t width, std::uint32_t height) 
        {
            create(width, height);
        }

        void create(std::uint32_t width, std::uint32_t height);

        std::uint32_t width() const noexcept { return myWidth; }
        std::uint32_t height() const noexcept { return myHeight; }

        bool inBounds(int x, int y) const noexcept;

        bool isOccupied(int x, int y) const noexcept;
        EntityId occupant(int x, int y) const noexcept;

        bool tryOccupy(int x, int y, EntityId e) noexcept;
        bool vacate(int x, int y, EntityId expected) noexcept;
        bool forceVacate(int x, int y) noexcept;

        bool move(EntityId e, int fromX, int fromY, int toX, int toY) noexcept;

        void clear() noexcept;

    private:
        std::size_t indexOf(int x, int y) const noexcept;

    private:
        std::uint32_t myWidth{0};
        std::uint32_t myHeight{0};

        std::vector<EntityId> myCells;
    };

} // namespace core