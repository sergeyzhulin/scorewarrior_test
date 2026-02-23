#pragma once
#include <Core/Map.h>
#include <Core/ECS/EntityId.h>

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <vector>

namespace features {

    struct NearestOccupied final {
        core::EntityId occupant{};
        int x{0};
        int y{0};

        bool isValid() const noexcept { return !occupant.isNull(); }
    };

    class MapUtils final {
    public:

        static void collectAdjacentOccupied(
		    const core::Map& map,
		    int x,
		    int y,
		    std::vector<core::EntityId>& out,
		    core::EntityId exclude = core::EntityId{}) noexcept 
        {

		    out.clear();

		    for (int dy = -1; dy <= 1; ++dy) {
			    for (int dx = -1; dx <= 1; ++dx) {
				    if (dx == 0 && dy == 0) {
					    continue;
				    }

				    const int nx = x + dx;
				    const int ny = y + dy;

				    if (!map.inBounds(nx, ny)) {
					    continue;
				    }

				    const core::EntityId occ = map.occupant(nx, ny);
				    if (occ.isNull()) {
					    continue;
				    }

				    if (!exclude.isNull() && occ == exclude) {
					    continue;
				    }

				    out.push_back(occ);
			    }
		    }
	    }

        static NearestOccupied findNearestOccupied(const core::Map& map, int fromX, int fromY) noexcept {
            NearestOccupied best{};
			double bestDist = std::numeric_limits<double>::max();

            const int w = static_cast<int>(map.width());
            const int h = static_cast<int>(map.height());

            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    const core::EntityId occ = map.occupant(x, y);
                    if (occ.isNull()) {
                        continue;
                    }

                    const double d = distance(fromX, fromY, x, y);
                    if (d < bestDist) {
                        bestDist = d;
                        best.occupant = occ;
                        best.x = x;
                        best.y = y;
                    }
                }
            }

            return best;
        }

		static double distance(int ax, int ay, int bx, int by) noexcept
		{
			const double dx = double(ax - bx);
			const double dy = double(ay - by);
			return std::sqrt(dx * dx + dy * dy);
		}
	};

} // namespace features