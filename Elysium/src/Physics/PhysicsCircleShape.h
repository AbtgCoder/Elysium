#pragma once

#include "PhysicsShape.h"

class PhysicsCircleShape : public PhysicsShape
{
public:
	PhysicsCircleShape();

	void ComputeMass(PhysicsMassData* massData, float density) const override;

public:
	// position (offset)
	Vec2 m_p;
};

inline PhysicsCircleShape::PhysicsCircleShape()
{
	m_type = e_circle;
	m_radius = 0.0f;
	m_p = Vec2(0.0f, 0.0f);
}