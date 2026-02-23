#include <Core/ECS/ComponentTypeRegistry.h>

#include <typeindex>
#include <unordered_map>
#include <vector>

namespace core 
{
    static std::unordered_map<std::type_index, size_t>& typeMap() 
    {
	    static std::unordered_map<std::type_index, size_t> s_map;
        return s_map;
    }

    static std::vector<std::string>& typeNames() 
    {
        static std::vector<std::string> s_names;
        return s_names;
    }

    size_t ComponentTypeRegistry::getIndexFor(const std::type_info& ti) 
    {
        auto& map = typeMap();
        auto& names = typeNames();

        const std::type_index key{ti};

        if (const auto it = map.find(key); it != map.end()) {
            return it->second;
        }

        const size_t idx = names.size();
        map.emplace(key, idx);
        names.emplace_back(key.name()); // just for debug
        return idx;
    }

} // namespace core