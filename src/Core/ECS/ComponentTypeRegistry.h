#pragma once
#include <typeinfo>
#include <span>
#include <string>

namespace core 
{

	// There are Component Types registry, to have type index
	class ComponentTypeRegistry final 
	{
	public:
		static size_t getIndexFor(const std::type_info& ti);
	};

	template <class T>
	struct ComponentType final {
		inline static const size_t index = ComponentTypeRegistry::getIndexFor(typeid(T));
	};

} // namespace core