#include "PhysicsWorld.h"

#include <iostream>

typedef std::map<ArbiterKey, Arbiter>::iterator ArbIter;
typedef std::pair<ArbiterKey, Arbiter> ArbPair;

PhysicsWorld::PhysicsWorld(const Vec2& gravity)
	: m_gravity(gravity), m_ImpulseIterations(10)
{

}

PhysicsWorld::PhysicsWorld(const Vec2& gravity, int iterations)
	: m_gravity(gravity), m_ImpulseIterations(iterations)
{
}

PhysicsWorld::~PhysicsWorld()
{
	Clear();
}

void PhysicsWorld::AddBody(PhysicsBody* body)
{
	m_bodies.push_back(body);
}

void PhysicsWorld::Clear()
{
	for (auto body : m_bodies) {
		delete body;
	}
	m_bodies.clear();
	m_arbiters.clear();
}

void PhysicsWorld::BroadPhase()
{
	// O(n^2) broad-phase
	for (size_t i = 0; i < m_bodies.size(); ++i)
	{
		PhysicsBody* bi = m_bodies[i];
		for (size_t j = i + 1; j < m_bodies.size(); ++j)
		{
			PhysicsBody* bj = m_bodies[j];
			if (bi->m_invMass == 0.0f && bj->m_invMass == 0.0f)
				continue;

			Arbiter newArb(bi, bj);
			ArbiterKey key(bi, bj);

			if (newArb.m_numContacts > 0)
			{
				ArbIter iter = m_arbiters.find(key);
				if (iter == m_arbiters.end())
				{
					m_arbiters.insert(ArbPair(key, newArb));
				}
				else
				{
					iter->second.UpdateContacts(newArb.m_contacts, newArb.m_numContacts);
				}
			}
			else
			{
				m_arbiters.erase(key);
			}
		}
	}
}

void PhysicsWorld::Step(float dt)
{
	float inv_dt = dt > 0.0f ? 1.0f / dt : 0.0f;

	// determine overlapping bodies and update contact points
	BroadPhase();

	// integrate forces
	for (int i = 0; i < (int)m_bodies.size(); i++)
	{
		PhysicsBody* b = m_bodies[i];

		if (b->m_type == PhysicsBodyType::staticBody)
			continue;

		b->m_velocity += (m_gravity + b->m_force * b->m_invMass) * dt;
		b->m_angularVelocity += b->m_torque * b->m_invI * dt;

	}

	

	// perform pre-steps
	for (ArbIter arb = m_arbiters.begin(); arb != m_arbiters.end(); ++arb)
	{
		arb->second.PreStep(inv_dt);
	}
	// perform Sequential Impulse
	for (int i = 0; i < m_ImpulseIterations; ++i)
	{
		for (ArbIter arb = m_arbiters.begin(); arb != m_arbiters.end(); ++arb)
		{
			arb->second.ApplyImpulse();
		}
	}

	// integrate velocities
	for (int i = 0; i < (int)m_bodies.size(); i++)
	{
		PhysicsBody* b = m_bodies[i];
		if (b->m_type == PhysicsBodyType::staticBody)
			continue;


		b->m_position += b->m_velocity * dt;
		b->m_rotation += b->m_angularVelocity * dt;

		b->m_force.Set(0.0f, 0.0f);
		b->m_torque = 0.0f;
	}

	
}


