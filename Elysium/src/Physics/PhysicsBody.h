#pragma once

#include "Math/Vec2.h"

#include "PhysicsShape.h"
#include "PhysicsCircleShape.h"
#include "PhysicsPolygonShape.h"


// TODO:  bodydef, fixturedef, fixtures ?? , drag ?? , NEED DEFS BECAUSE WHO OWNS POINTERS ??, THEIR CREATION AND THEIR DELETION, THIS ALL SHOULD BE HANDLED BE ENGINE USING EFFICIENT MEMORY ALLOCATION

//TODO: kinematic body ??
enum PhysicsBodyType
{
	staticBody = 0,
	dynamicBody = 1
};

class PhysicsBody
{
public:
	PhysicsBody();
	void ResetMassData(float d);

	void AddForce(const Vec2& f);

	PhysicsShape* GetShape();
	PhysicsShape::Type GetShapeType() const;

public:
	Vec2 m_position;
	float m_rotation;
	Vec2 m_velocity;
	float m_angularVelocity;
		
	PhysicsBodyType m_type;

	Vec2 m_force;
	float m_torque;

	PhysicsShape* m_shape;

	float m_friction;
	float m_density;

	float m_mass, m_invMass;
	float m_I, m_invI;

	friend class Arbiter;
	friend class PhysicsWorld;
};