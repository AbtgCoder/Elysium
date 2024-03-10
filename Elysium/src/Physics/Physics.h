#pragma once
#include "Level/Entity.h"

class Physics
{
public:
	Physics() {};

	static Vec2 GetOverlap(Entity a, Entity b);
	static Vec2 GetPreviousOverlap(Entity a, Entity b);

};