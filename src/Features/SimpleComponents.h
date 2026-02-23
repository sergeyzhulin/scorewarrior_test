#pragma once

namespace features {

struct UnitId 
{ 
	uint32_t value{0}; 
};

struct Health
{
	uint32_t value{0};
};

struct Power
{
	uint32_t value{0};
};

struct Strength
{
	uint32_t value{0};
};

struct Agility
{
	uint32_t value{0};
};

struct Range
{
	uint32_t minDistance{0};
	uint32_t maxDistance{0};
};

struct Spirit
{
	uint32_t value{0};
};	

struct Movable
{
	uint32_t step{1};
};

struct Flyable
{
	uint32_t step{1};
};

struct TargetPos
{
	uint32_t x{0};
	uint32_t y{0};
	bool go = false;
};	


//Raven can decrease range

} // namespace features
