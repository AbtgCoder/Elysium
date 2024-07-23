#pragma once

#include "Math/Vec2.h"

#include "PhysicsBodyPairArbiter.h"
#include "PhysicsHingeJoint.h"

#include <map>

class PhysicsWorld
{
public:
	PhysicsWorld(const Vec2& gravity);
	PhysicsWorld(const Vec2& gravity, int iterations);
	~PhysicsWorld();

	void AddBody(PhysicsBody* body);
	void AddJoint(PhysicsHingeJoint* joint);

	void Clear();

	void Step(float dt);

	void BroadPhase();
public:
	std::vector<PhysicsBody*> m_bodies;
	std::map<ArbiterKey, Arbiter> m_arbiters;

	std::vector<PhysicsHingeJoint*> m_joints;

	Vec2 m_gravity = {0.0f, 9.8f};
	int m_ImpulseIterations = 10;
	bool accumulateImpulses = true;
	bool warmStarting = true;
};