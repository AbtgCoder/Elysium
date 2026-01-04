#pragma once

#include "Math/Vec2.h"

#include "PhysicsBodyPairArbiter.h"
#include "PhysicsHingeJoint.h"

#include <map>

//TODO: maybe should be in a separate file
struct ContactEvent
{
	PhysicsBody* bodyA = nullptr;
	PhysicsBody* bodyB = nullptr;
	Vec2 contactPoint = { 0.0f, 0.0f };
	Vec2 contactNormal = { 0.0f, 0.0f };
	float separation = 0.0f;
	float normalImpulse = 0.0f;	// post-solve normal impulse magnitude
	float tangentImpulse = 0.0f;	// post-solve normal impulse magnitude
};

struct ContactListener
{
	virtual ~ContactListener() = default;
	virtual void OnContactBegin(const ContactEvent& event) {}
	virtual void OnContactEnd(const ContactEvent& event) {}
	virtual void OnContactPreSolve(const ContactEvent& event) {}
	virtual void OnContactPostSolve(const ContactEvent& event) {}
};

class PhysicsWorld
{
public:
	PhysicsWorld(const Vec2& gravity);
	PhysicsWorld(const Vec2& gravity, int iterations);
	~PhysicsWorld();

	void AddBody(PhysicsBody* body);
	void AddJoint(PhysicsHingeJoint* joint);

	void Clear();

	// step once with variable timestep; uses internal accumulator to call fixed timestep substeps
	void Update(float dt);

	// internal fixed timestep step
	void Step(float fixedDt);


	void SetContactListener(ContactListener* listener) { m_contactListener = listener; }
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

	void NotifyContactBegin(const Arbiter& arb);
	void NotifyContactEnd(const ArbiterKey& key);
	void NotifyContactPreSolve(const Arbiter& arb);
	void NotifyContactPostSolve(const Arbiter& arb);
private:
	float m_accumulator = 0.0f; // for variable timestep handling

	ContactListener* m_contactListener = nullptr;
	// track previous frame contacts to generate contact begin/end events
	std::map<ArbiterKey, bool> m_previousContacts;
};