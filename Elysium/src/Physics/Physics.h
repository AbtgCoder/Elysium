#pragma once
//#include "Scene/Entity.h"

#include "KDTree.h"

class Physics
{
public:
	Physics() {};

	static std::vector<Vec2> AABBCollision(Entity a, Entity b);
	static Vec2 GetOverlap(Entity a, Entity b);
	static Vec2 GetPreviousOverlap(Entity a, Entity b);

	static std::vector<Vec2> SAT(Entity a, Entity b);
	static bool CircleCircleCollision(Entity a, Entity b);

	static void NarrowPhaseCollision(KDTreeNode* node);
};