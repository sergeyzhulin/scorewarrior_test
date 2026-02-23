#pragma once
#include <Core/ActionId.h>
#include <Core/FilterId.h>

#include <vector>
#include <cstddef>
#include <cassert>

namespace core 
{

    class ActionFilters final {
    public:
        void set(ActionId actionId, FilterId filterId) 
        {
            assert(!actionId.isNull());
            ensureCapacity(actionId);
            myByAction[actionId.value()] = filterId;
        }

        FilterId get(ActionId actionId) const noexcept 
        {
            if (actionId.isNull()) {
                return FilterId{};
            }
            const std::size_t idx = actionId.value();
            if (idx >= myByAction.size()) {
                return FilterId{};
            }
            return myByAction[idx];
        }

        void clear(ActionId actionId) {
            assert(!actionId.isNull());
            const std::size_t idx = actionId.value();
            if (idx < myByAction.size()) {
                myByAction[idx] = FilterId{};
            }
        }

    private:
        void ensureCapacity(ActionId actionId) 
        {
            const std::size_t wanted = actionId.value() + 1;
            if (myByAction.size() < wanted) {
                myByAction.resize(wanted, FilterId{});
            }
        }

    private:
        std::vector<FilterId> myByAction; // used as map [actionId] -> filterId
    };

} // namespace core