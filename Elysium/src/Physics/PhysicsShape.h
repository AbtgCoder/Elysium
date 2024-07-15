#pragma once

#include "Math/Vec2.h"

struct PhysicsMassData
{
	float mass; // in Kgs
	Vec2 center; // offset of shape relative to origin
	float I; // rotational inertia of shape about local origin
};

class PhysicsShape
{
public:
	//TODO: add more shapes (edge ...)
	enum Type
	{
		e_circle = 0,
		e_polygon = 1,
	};

	virtual ~PhysicsShape() {}

	Type GetType() const;

	virtual void ComputeMass(PhysicsMassData* massData, float density) const = 0;

public:
	Type m_type;
	float m_radius;
};

inline PhysicsShape::Type PhysicsShape::GetType() const
{
	return m_type;
}