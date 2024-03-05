#pragma once
#include "Level/Entity.h"

class Physics
{
public:
	Physics() {};

	static Vec2 GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
	static Vec2 GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);

};