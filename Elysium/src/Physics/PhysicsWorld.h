#pragma once

#include "Math/Vec2.h"

class PhysicsWorld
{
public:
	PhysicsWorld(const Vec2& gravity);
	~PhysicsWorld() = default;


private:
	Vec2 m_gravity;
};