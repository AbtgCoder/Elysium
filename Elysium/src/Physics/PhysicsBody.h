#pragma once

#include "Math/Vec2.h"

#include "PhysicsShape.h"
#include "PhysicsCircleShape.h"
#include "PhysicsPolygonShape.h"


// TODO:  bodydef, fixturedef, fixtures ?? , drag ?? , NEED DEFS BECAUSE WHO OWNS POINTERS ??, THEIR CREATION AND THEIR DELETION, THIS ALL SHOULD BE HANDLED BE ENGINE USING EFFICIENT MEMORY ALLOCATION

struct AABB
{
	Vec2 min;
	Vec2 max;
};


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

	PhysicsShape* GetShape();
	PhysicsShape::Type GetShapeType() const;

	void ComputeAABB();

	// API
	void ApplyForceToCenter(const Vec2& f) { if (m_type != PhysicsBodyType::dynamicBody) return; m_force += f; WakeUp(); }
	void ApplyForceToPoint(const Vec2& f, const Vec2& point);
	void ApplyImpulseToCenter(const Vec2& impulse);
	void ApplyImpulseToPoint(const Vec2& impulse, const Vec2& point);
	void ClearForces() { m_force.Set(0.0f, 0.0f); m_torque = 0.0f; }

	// sleeping helpers
	void SetSleeping(bool sleep) { m_isSleeping = sleep; if (sleep) { m_velocity.Set(0, 0); m_angularVelocity = 0.0f; m_force.Set(0, 0); m_torque = 0; } }
	void WakeUp() { m_isSleeping = false; m_sleepTime = 0.0f; }
	bool IsSleeping() const { return m_isSleeping; }
public:
	Vec2 m_position = {0.0f, 0.0f};
	float m_rotation = 0.0f;
	Vec2 m_velocity = {0.0f, 0.0f};
	float m_angularVelocity = 0.0f;
		
	PhysicsBodyType m_type = PhysicsBodyType::dynamicBody;

	Vec2 m_force = {0.0f, 0.0f};
	float m_torque = 0.0f;

	PhysicsShape* m_shape;

	AABB m_aabb;

	float m_friction;
	float m_restitution;
	float m_restitutionThreshold;
	float m_density;

	float m_mass = 1.0f, m_invMass = 1.0f;
	float m_I = 1.0f, m_invI = 1.0f;

	// sleep
	bool m_isSleeping = false;
	float m_sleepTime = 0.0f;

	friend struct Arbiter;
	friend class PhysicsWorld;
};