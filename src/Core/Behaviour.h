#pragma once
#include <Core/ActionId.h>

#include <vector>
#include <cassert>

namespace core 
{

    class Behaviour final 
    {
    public:
        void push_back(ActionId id) 
        {
            assert(!id.isNull());
            myActions.push_back(id);
        }

        const std::vector<ActionId>& actions() const noexcept { return myActions; }

    private:
        std::vector<ActionId> myActions;
    };

} // namespace core