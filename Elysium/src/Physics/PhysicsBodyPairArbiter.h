#pragma once

#include "PhysicsBody.h"


//  the features that intersect to form the contact point
struct ContactFeature
{
	enum Type
	{
		e_vertex = 0,
		e_face = 1
	};

	uint8_t indexA; // feature index on shapeA
	uint8_t indexB; // feature index on shapeB
	uint8_t typeA;  // feature type on shapeA
	uint8_t typeB;  // feature type on shapeB
};

union ContactID
{
	ContactFeature cf;
	uint32_t key; // used to quickly compare contact ids
};

struct Contact
{
	Contact() {}

	Vec2 m_position = {0.0f, 0.0f};
	Vec2 m_normal = {0.0f, 0.0f};
	Vec2 m_r1 = { 0.0f, 0.0f }, m_r2 = { 0.0f, 0.0f };
	float m_separation = 0.0f;
	float m_Jn = 0.0f; // accumulated normal impulse
	float m_Jt = 0.0f; // accumulated tangent impulse
	float m_Jnb = 0.0f; // accumulated normal impulse for position bias
	float m_massNormal = 0.0f, m_massTangent = 0.0f;
	float m_bias = 2.0f;
	// TODO: contact ID
	ContactID m_id;
};

struct ArbiterKey
{
	ArbiterKey(PhysicsBody* b1, PhysicsBody* b2)
	{
		if (b1 < b2)
		{
			m_body1 = b1; m_body2 = b2;
		}
		else
		{
			m_body1 = b2; m_body2 = b1;
		}
	}

	PhysicsBody* m_body1;
	PhysicsBody* m_body2;
};

struct Arbiter
{
	enum {MAX_POINTS = 2};

	Arbiter(PhysicsBody* b1, PhysicsBody* b2);

	void UpdateContacts(Contact* newContacts, int numNewContacts);

	void PreStep(float inv_dt);
	void ApplyImpulse();

	Contact m_contacts[MAX_POINTS];
	int m_numContacts;

	PhysicsBody* m_body1;
	PhysicsBody* m_body2;

	// combined friction: root(f1*f2)
	float m_friction;
	float m_restitution; // combined restitution: max of e1,e2
	float m_restitionThreshold; // combined restitution threshold: min of threshold1 & threshold2
};

inline bool operator < (const ArbiterKey& a1, const ArbiterKey& a2)
{
	if (a1.m_body1 < a2.m_body1)
		return true;
	if (a1.m_body1 == a2.m_body1 && a1.m_body2 < a2.m_body2)
		return true;

	return false;
}

int Collide(Contact* contacts, PhysicsBody* body1, PhysicsBody* body2);