#include "PhysicsWorld.h"

#include <iostream>
#include <set>
#include <core/Logger.h>

typedef std::map<ArbiterKey, Arbiter>::iterator ArbIter;
typedef std::pair<ArbiterKey, Arbiter> ArbPair;


// internal helper for SAP broad-phase
struct SweepEntry
{
	PhysicsBody* body;
	float minX;
	float maxX;
};

// build a sweep list from current bodies' AABBs
static void BuildSweepList(const std::vector<PhysicsBody*>& bodies, std::vector<SweepEntry>& sweepList)
{
	sweepList.clear();
	sweepList.reserve(bodies.size());

	for (size_t i = 0; i < bodies.size(); ++i)
	{
		PhysicsBody* b = bodies[i];
		if (!b || !b->m_shape)
			continue;

		SweepEntry entry;
		entry.body = b;
		entry.minX = b->m_aabb.min.x;
		entry.maxX = b->m_aabb.max.x;
		sweepList.push_back(entry);
	}

	// use insertion sort(better for almost sorted data, as bodies dont move much from frame to frame) to sort sweep list by minX
	for (size_t i = 1; i < sweepList.size(); ++i)
	{
		SweepEntry key = sweepList[i];
		int j = i;
		while (j > 0 && sweepList[j - 1].minX > key.minX)
		{
			sweepList[j] = sweepList[j - 1];
			--j;
		}
		sweepList[j] = key;
	}
}

// exact AABB overlap test
static inline bool AABBOverlap(const PhysicsBody* a, const PhysicsBody* b)
{
	return (a->m_aabb.min.x <= b->m_aabb.max.x && a->m_aabb.max.x >= b->m_aabb.min.x) &&
		(a->m_aabb.min.y <= b->m_aabb.max.y && a->m_aabb.max.y >= b->m_aabb.min.y);
}


PhysicsWorld::PhysicsWorld(const Vec2& gravity)
	: m_gravity(gravity), m_ImpulseIterations(10)
{
	m_fixedTimestep = 1.0f / 60.0f;
	m_maxSubSteps = 5;
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

void PhysicsWorld::DestroyBody(PhysicsBody* body)
{
	if (!body)
		return;

	// remove joints involving this body
	for (auto it = m_joints.begin(); it != m_joints.end();)
	{
		PhysicsHingeJoint* joint = *it;
		if (joint->m_body1 == body || joint->m_body2 == body)
		{
			delete joint;
			it = m_joints.erase(it);
		}
		else
		{
			++it;
		}
	}

	// remove body from arbiters
	for (auto it = m_arbiters.begin(); it != m_arbiters.end();)
	{
		const ArbiterKey& key = it->first;

		if (key.m_body1 == body || key.m_body2 == body)
		{
			// Notify collision end before removal ig...
			NotifyCollisionEnd(key);

			// remove from previous collisions map
			m_previousCollisions.erase(key);

			it = m_arbiters.erase(it);
		}
		else
		{
			++it;
		}
	}

	// remove from bodies vector
	auto it = std::find(m_bodies.begin(), m_bodies.end(), body);
	if (it != m_bodies.end())
		m_bodies.erase(it);

	// delete body
	delete body;
}

void PhysicsWorld::AddJoint(PhysicsHingeJoint* joint)
{
	m_joints.push_back(joint);
}

void PhysicsWorld::DestroyJoint(PhysicsHingeJoint* joint)
{
	if (!joint)
		return;

	auto it = std::find(m_joints.begin(), m_joints.end(), joint);
	if (it != m_joints.end())
		m_joints.erase(it);

	delete joint;
}


void PhysicsWorld::Clear()
{
	for (auto body : m_bodies) {
		delete body;
	}
	m_bodies.clear();
	
	for (auto joint : m_joints) {
		delete joint;
	}
	m_joints.clear();
	
	m_arbiters.clear();

	m_previousCollisions.clear();

	delete m_CollisionListener;//TODO: maybe shouldn't be doing this here...
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

// sweep and prune broad-phase implementation
void PhysicsWorld::BroadhPhaseSAP()
{
	// 1) build sorted sweep list on X axis
	std::vector<SweepEntry> sweepList;
	BuildSweepList(m_bodies, sweepList);

	// 2) generate candidate pairs by sweeping
	std::vector<std::pair<PhysicsBody*, PhysicsBody*>> candidatePairs;
	candidatePairs.reserve(sweepList.size() * sweepList.size() / 4); // rough estimate

	const size_t n = sweepList.size();
	for (size_t i=0; i < n; ++i)
	{
		const SweepEntry& entryA = sweepList[i];

		if (!entryA.body || !entryA.body->m_shape)
			continue;

		for (size_t j = i + 1; j < n; ++j)
		{
			const SweepEntry& entryB = sweepList[j];

			// if b.minx > a.maxx then all following bs will also be too far right so break inner loop
			if (entryB.minX > entryA.maxX)
				break;

			// quick y-axis cull (full AABB overlap check)
			if (!AABBOverlap(entryA.body, entryB.body))
				continue;

			// we now have a candidate pair
			candidatePairs.emplace_back(entryA.body, entryB.body);
		}
	}

	// 3) narrow phase and update arbiters from candidate pairs
	UpdateArbitersFromCandidates(candidatePairs);
}




// update arbiters from candidate body pairs produed by SAP
// marks touched arbiters via a set of keys and remove arbiters that were not touched
void PhysicsWorld::UpdateArbitersFromCandidates(const std::vector<std::pair<PhysicsBody*, PhysicsBody*>>& candidates)
{
	// track which arbiters were touched this frame
	std::set<ArbiterKey> touchedKeys;

	// iterate candidate pairs, run narrowphase (arbiter construtor does shape tests & fills contacts)
	for (const auto& pair : candidates)
	{
		PhysicsBody* b1 = pair.first;
		PhysicsBody* b2 = pair.second;

		if (b1->m_invMass == 0.0f && b2->m_invMass == 0.0f)
			continue;

		Arbiter newArb(b1, b2);
		ArbiterKey key(b1, b2);


		if (newArb.m_numContacts > 0)
		{
			touchedKeys.insert(key);
			
			ArbIter iter = m_arbiters.find(key);
			if (iter == m_arbiters.end())
			{
				m_arbiters.emplace(key, std::move(newArb)); // insert new arbiter
			}
			else
			{
				iter->second.UpdateContacts(newArb.m_contacts, newArb.m_numContacts); // existing arbiter: update contacts
			}
		}
		else 
		{
			// no contacts: we dont insert a touched key
		}
	}

	// remove arbiters that were not touched this frame
	for (auto it = m_arbiters.begin(); it != m_arbiters.end(); )
	{
		if (touchedKeys.find(it->first) == touchedKeys.end())
		{
			it = m_arbiters.erase(it);
		}
		else
		{
			++it;
		}
	}
}


// Helper: create collision event from arbiter
static CollisionEvent CreateCollisionEvent(const Arbiter& arb)
{
	CollisionEvent event;
	event.bodyA = arb.m_body1;
	event.bodyB = arb.m_body2;

	event.numContacts = arb.m_numContacts;

	event.contacts.resize(event.numContacts);

	for (int i = 0; i < event.numContacts; i++)
	{
		event.contacts[i].position = arb.m_contacts[i].m_position;
		event.contacts[i].normal = arb.m_contacts[i].m_normal;
		event.contacts[i].penetration = -arb.m_contacts[i].m_separation;
	}

	return event;
}

void PhysicsWorld::NotifyCollisionBegin(const Arbiter& arb)
{
	if (!m_CollisionListener)
		return;

	m_CollisionListener->OnCollisionBegin(CreateCollisionEvent(arb));
}

void PhysicsWorld::NotifyCollisionStay(const Arbiter& arb)
{
	if (!m_CollisionListener)
		return;

	m_CollisionListener->OnCollisionBegin(CreateCollisionEvent(arb));
}

void PhysicsWorld::NotifyCollisionEnd(const ArbiterKey& key)
{
	if (!m_CollisionListener)
		return;
	CollisionEvent event;
	event.bodyA = key.m_body1;
	event.bodyB = key.m_body2;
	m_CollisionListener->OnCollisionEnd(event);
}


void PhysicsWorld::Update(float dt)
{
	// clamp to avoid spiral of death
	const float maxDt = m_fixedTimestep * m_maxSubSteps;
	if (dt > maxDt)
		dt = maxDt;	

	m_accumulator += dt;
	int subSteps = 0;
	while (m_accumulator >= m_fixedTimestep && subSteps < m_maxSubSteps)
	{
		Step(m_fixedTimestep);
		m_accumulator -= m_fixedTimestep;
		subSteps++;
	}

	//TODO: remainingg accumulator can be used for interpolation ??
}

void PhysicsWorld::Step(float dt)
{
	float inv_dt = dt > 0.0f ? 1.0f / dt : 0.0f;

	// 1) integrate forces (apply gravity + explicit per-body forces) into velocities (semi-implicit euler)
	for (int i = 0; i < (int)m_bodies.size(); i++)
	{
		PhysicsBody* b = m_bodies[i];

		if (!b || b->m_type != PhysicsBodyType::dynamicBody || b->IsSleeping())
			continue;

		b->m_velocity += (m_gravity + b->m_force * b->m_invMass) * dt;
		b->m_angularVelocity += b->m_torque * b->m_invI * dt;
		
		//TODO: damping ??
	}



	// 2) determine overlapping bodies and update contact points
	
	// compute AABBs for all bodies
	for (int i = 0; i < (int)m_bodies.size(); i++)
	{
		PhysicsBody* b = m_bodies[i];
		if (!b || !b->m_shape)
			continue;
		b->ComputeAABB();
	}

	BroadhPhaseSAP();

	// 3) handle contact begin/end events
	// build current contact presence map
	std::map<ArbiterKey, bool> currentContacts;
	for (auto& arbPair : m_arbiters)
	{
		const ArbiterKey& key = arbPair.first;
		const Arbiter& arb = arbPair.second;

		currentContacts[key] = true;

		auto prevIt = m_previousCollisions.find(key);

		// begin contact: present now but not in previous frame
		if (prevIt == m_previousCollisions.end())
		{
			NotifyCollisionBegin(arb);
		}
		else // contact continues
		{
			NotifyCollisionStay(arb);
		}
	}

	// end contact: present in previous frame but not now
	for (auto& prevContact : m_previousCollisions)
	{
		if (currentContacts.find(prevContact.first) == currentContacts.end())
		{
			NotifyCollisionEnd(prevContact.first);
		}
	}

	// swap contact maps for next frame
	m_previousCollisions = std::move(currentContacts);


	// 4) solve contacts and constraints
	
	// perform pre-steps
	for (ArbIter arb = m_arbiters.begin(); arb != m_arbiters.end(); ++arb)
	{
		arb->second.PreStep(inv_dt);
	}
	for (int i = 0; i < (int)m_joints.size(); i++)
	{
		m_joints[i]->PreStep(inv_dt);
	}
	// perform Sequential Impulse
	for (int i = 0; i < m_ImpulseIterations; ++i)
	{
		for (ArbIter arb = m_arbiters.begin(); arb != m_arbiters.end(); ++arb)
		{
			arb->second.ApplyImpulse();
		}

		for (int i = 0; i < (int)m_joints.size(); i++)
		{
			m_joints[i]->ApplyImpulse();
		}
	}


	// 5) integrate velocities to positions
	for (int i = 0; i < (int)m_bodies.size(); i++)
	{
		PhysicsBody* b = m_bodies[i];
		if (!b || b->m_type == PhysicsBodyType::staticBody)
			continue;

		if (b->IsSleeping())
		{
			// clear forces even if sleeping
			b->ClearForces();
			continue;
		}

		b->m_position += b->m_velocity * dt;
		b->m_rotation += b->m_angularVelocity * dt;

		// clear accumulated forces for next step
		b->ClearForces();
	}

	// 6) sleep check: detect low motion bodies and put to sleep
	for (int i = 0; i < (int)m_bodies.size(); i++)
	{
		PhysicsBody* b = m_bodies[i];
		if (!b || b->m_type != PhysicsBodyType::dynamicBody)
			continue;
		if (b->m_velocity.length() < m_sleepLinearThreshold &&
			std::abs(b->m_angularVelocity) < m_sleepAngularThreshold)
		{
			b->m_sleepTime += dt;
			if (b->m_sleepTime >= m_timeToSleep)
			{
				b->SetSleeping(true);
			}
		}
		else
		{
			b->m_sleepTime = 0.0f;
			if (b->IsSleeping())
			{
				b->WakeUp();
			}
		}
	}
	
	// 7) wake bodies involved in contacts
	for (ArbIter arb = m_arbiters.begin(); arb != m_arbiters.end(); ++arb)
	{
		PhysicsBody* b1 = arb->second.m_body1;
		PhysicsBody* b2 = arb->second.m_body2;
		if (!b1 || !b2)
			continue;
		if (b1->IsSleeping() && (b2->m_type != PhysicsBodyType::staticBody))
			b1->WakeUp();
		if (b2->IsSleeping() && (b1->m_type != PhysicsBodyType::staticBody))
			b2->WakeUp();
	}
}


