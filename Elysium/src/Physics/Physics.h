#pragma once
//#include "Level/Entity.h"

#include "KDTree.h"

class Physics
{
public:
	Physics() {};

	static Vec2 GetOverlap(Entity a, Entity b);
	static Vec2 GetPreviousOverlap(Entity a, Entity b);

	static bool SAT(Entity a, Entity b);
	static bool CircleCircleCollision(Entity a, Entity b);

	static void NarrowPhaseCollision(KDTreeNode* node);
};