#pragma once

#include "Math/Vec2.h"

#include "PhysicsBodyPairArbiter.h"
#include "PhysicsHingeJoint.h"

#include <map>

//TODO: maybe should be in a separate file
struct ContactPoint2D
{
	Vec2 position = { 0.0f, 0.0f };
	Vec2 normal = { 0.0f, 0.0f };
	float penetration = 0.0f;
};
struct CollisionEvent
{
	PhysicsBody* bodyA = nullptr;
	PhysicsBody* bodyB = nullptr;
	std::vector<ContactPoint2D> contacts;
	size_t numContacts = 0;
};

struct CollisionListener
{
	virtual ~CollisionListener() = default;
	virtual void OnCollisionBegin(const CollisionEvent& event) {}
	virtual void OnCollisionStay(const CollisionEvent& event) {}
	virtual void OnCollisionEnd(const CollisionEvent& event) {}
};

class PhysicsWorld
{
public:
	PhysicsWorld(const Vec2& gravity);
	PhysicsWorld(const Vec2& gravity, int iterations);
	~PhysicsWorld();

	
	void AddBody(PhysicsBody* body);
	void DestroyBody(PhysicsBody* body);
	void AddJoint(PhysicsHingeJoint* joint);
	void DestroyJoint(PhysicsHingeJoint* joint);

	void Clear();

	// step once with variable timestep; uses internal accumulator to call fixed timestep substeps
	void Update(float dt);

	// internal fixed timestep step
	void Step(float fixedDt);


	void SetContactListener(CollisionListener* listener) { m_CollisionListener = listener; }
public:
	std::vector<PhysicsBody*> m_bodies;
	std::map<ArbiterKey, Arbiter> m_arbiters;

	std::vector<PhysicsHingeJoint*> m_joints;

	Vec2 m_gravity = {0.0f, 9.8f};
	int m_ImpulseIterations = 10;
	bool accumulateImpulses = true;
	bool warmStarting = true;

	float m_fixedTimestep = 1.0f / 60.0f;
	int m_maxSubSteps = 5;

	// sleep config
	float m_sleepLinearThreshold = 0.01f; // units/s
	float m_sleepAngularThreshold = 2.0f / 180.0f * 3.1415926f; // radians/s
	float m_timeToSleep = 0.5f; // seconds of low motion before sleep

	// position correction config
	float m_positionCorrectionPercent = 0.2f; // baumgarte position correction percentage
	float m_positionCorrectionSlop = 0.01f; // penetration allowance before correction

private:
	void BroadPhase();
	void BroadhPhaseSAP();
	void UpdateArbitersFromCandidates(const std::vector<std::pair<PhysicsBody*, PhysicsBody*>>& candidates);

	void NotifyCollisionBegin(const Arbiter& arb);
	void NotifyCollisionStay(const Arbiter& arb);
	void NotifyCollisionEnd(const ArbiterKey& key);
private:
	float m_accumulator = 0.0f; // for variable timestep handling

	CollisionListener* m_CollisionListener = nullptr;
	// track previous frame contacts to generate contact begin/end events
	std::map<ArbiterKey, bool> m_previousCollisions;
};