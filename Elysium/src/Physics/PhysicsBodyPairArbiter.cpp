#include "PhysicsBodyPairArbiter.h"
#include <algorithm>
#include <iostream>


Arbiter::Arbiter(PhysicsBody* b1, PhysicsBody* b2)
{
	if (b1 < b2)
	{
		m_body1 = b1;
		m_body2 = b2;
	}
	else
	{
		m_body1 = b2;
		m_body2 = b1;
	}

	//m_body1 = b1;
	//m_body2 = b2;

	m_numContacts = Collide(m_contacts, m_body1, m_body2);

	m_friction = sqrtf(m_body1->m_friction * m_body2->m_friction);
	m_restitution = std::max(m_body1->m_restitution, m_body2->m_restitution);
	m_restitionThreshold = std::min(m_body1->m_restitutionThreshold, m_body2->m_restitutionThreshold);
}

#if 0
void Arbiter::UpdateContacts(Contact* newContacts, int newCount)
{
	const float kMatchEpsilon = 0.01f;
	Contact old[100];
	int oldCount = m_numContacts;
	for (int i = 0; i < oldCount; ++i) old[i] = m_contacts[i];

	// copy new contacts into m_contacts, but try to copy impulses from old contacts
	for (int i = 0; i < newCount; ++i)
	{
		Contact nc = newContacts[i];
		nc.m_Jn = 0.0f;
		nc.m_Jt = 0.0f;
		// try to find match
		for (int j = 0; j < oldCount; ++j)
		{
			if (nc.m_position.dist(old[j].m_position) < kMatchEpsilon)
			{
				nc.m_Jn = old[j].m_Jn;
				nc.m_Jt = old[j].m_Jt;
				break;
			}
		}
		m_contacts[i] = nc;
	}
	m_numContacts = newCount;
}
#endif

void Arbiter::UpdateContacts(Contact* newContacts, int numNewContacts)
{
	Contact mergedContacts[2];

	for (int i = 0; i < numNewContacts; ++i)
	{
		Contact* cNew = newContacts + i;
		int k = -1;
		for (int j = 0; j < m_numContacts; ++j)
		{
			Contact* cOld = m_contacts + j;
			if (cNew->m_id.key == cOld->m_id.key)
			{
				k = j;
				break;
			}
		}

		if (k > -1)
		{
			Contact* c = mergedContacts + i;
			Contact* cOld = m_contacts + k;
			*c = *cNew;

			c->m_Jn = cOld->m_Jn;
			c->m_Jt = cOld->m_Jt;
			c->m_Jnb = cOld->m_Jnb;
		}
		else
		{
			mergedContacts[i] = newContacts[i];
		}

		if (k == -1) {
			//std::cout << "Contact replaced: old key=" << m_contacts[0].m_id.key << " newKey=" << cNew->m_id.key << "\n";
		}

	}

	for (int i = 0; i < numNewContacts; ++i)
		m_contacts[i] = mergedContacts[i];

	m_numContacts = numNewContacts;
}


void Arbiter::PreStep(float inv_dt)
{
	const float k_allowedPenetration = 0.01f;
	float k_biasFactor = 0.2f; 

	// for each contact
	for (int i = 0; i < m_numContacts; i++)
	{
		Contact* c = m_contacts + i;

		Vec2 r1 = c->m_position - m_body1->m_position;
		Vec2 r2 = c->m_position - m_body2->m_position;

		// precompute normal mass, tangent mass and bias
		float rn1 = r1.dot(c->m_normal);
		float rn2 = r2.dot(c->m_normal);
		float kNormal = m_body1->m_invMass + m_body2->m_invMass;
		kNormal += m_body1->m_invI * (r1.dot(r1) - rn1 * rn1) + m_body2->m_invI * (r2.dot(r2) - rn2 * rn2); //NOTE: this only works if c->normal is a unit vector
		c->m_massNormal = 1.0f / kNormal;

		Vec2 tangent = { c->m_normal.y, -c->m_normal.x };
		float rt1 = r1.dot(tangent);
		float rt2 = r2.dot(tangent);
		float kTangent = m_body1->m_invMass + m_body2->m_invMass;
		kTangent += m_body1->m_invI * (r1.dot(r1) - rt1 * rt1) + m_body2->m_invI * (r2.dot(r2) - rt2 * rt2); 
		c->m_massTangent = 1.0f / kTangent;

		//NOTE: add bias velocity (proportional to penetration) to give normal impulse some extra oomph!!
		c->m_bias = -1 * k_biasFactor * inv_dt * std::min(0.0f, c->m_separation + k_allowedPenetration);
		//ESM_LOG("bias", c->m_bias);
		//c->m_bias = 0.0f;
		
		//if accumulate impulses then:
		{
			// Apply normal + frictional impulse
			Vec2 J = c->m_normal * c->m_Jn + tangent * c->m_Jt;
			//ESM_LOG("prestep impulse", J);
			m_body1->m_velocity -= J * m_body1->m_invMass;
			m_body1->m_angularVelocity -= (r1.x * J.y - r1.y * J.x) * m_body1->m_invI;
			
			m_body2->m_velocity += J * m_body2->m_invMass;
			m_body2->m_angularVelocity += (r2.x * J.y - r2.y * J.x) * m_body2->m_invI;
		}
	}

	//TODO: block solver
	if (m_numContacts == 2)
	{
		Contact* c1 = m_contacts;
		Contact* c2 = m_contacts + 1;
		Vec2 r1A = c1->m_position - m_body1->m_position;
		Vec2 r1B = c1->m_position - m_body2->m_position;
		Vec2 r2A = c2->m_position - m_body1->m_position;
		Vec2 r2B = c2->m_position - m_body2->m_position;
		float rn1A = Cross(r1A, c1->m_normal);
		float rn1B = Cross(r1B, c1->m_normal);
		float rn2A = Cross(r2A, c1->m_normal);
		float rn2B = Cross(r2B, c1->m_normal);

		float k11;
		float k22;
		float k12;

	}

	//TODO: do warm starting later ??
}

void Arbiter::ApplyImpulse()
{
	PhysicsBody* b1 = m_body1;
	PhysicsBody* b2 = m_body2;


	std::vector<Vec2> normalImpulses;
	std::vector<Vec2> tangentialImpulses;


	for (int i = 0; i < m_numContacts; i++)
	{
		Contact* c = m_contacts + i;
		c->m_r1 = c->m_position - b1->m_position;
		c->m_r2 = c->m_position - b2->m_position;

		// relative velocity at contact
		Vec2 dv = b2->m_velocity + Cross(b2->m_angularVelocity, c->m_r2) - b1->m_velocity - Cross(b1->m_angularVelocity, c->m_r1);

		// compute normal impulse
		float vn = dv.dot(c->m_normal);
		float dJn = c->m_massNormal * (-vn + c->m_bias);

		//ESM_LOG("vn", vn, "djn", dJn);

		// clamp the accumulated impulse:
		float Jn0 = c->m_Jn;
		c->m_Jn = std::max(Jn0 + dJn, 0.0f);
		dJn = c->m_Jn - Jn0;

		Vec2 Jn = c->m_normal * dJn;

		normalImpulses.push_back(Jn);

		//	ESM_LOG("collision normal", c->m_normal, "new velocities: ", b1->m_velocity, b2->m_velocity);

	}

	for (int i = 0; i < m_numContacts; i++)
	{
		Contact* c = m_contacts + i;
		Vec2 Jn = normalImpulses[i];
		b1->m_velocity -= Jn * b1->m_invMass;
		b1->m_angularVelocity -= Cross(c->m_r1, Jn) * b1->m_invI;
		b2->m_velocity += Jn * b2->m_invMass;
		b2->m_angularVelocity += Cross(c->m_r2, Jn) * b2->m_invI;
	}

	for (int i = 0; i < m_numContacts; i++)
	{
		Contact* c = m_contacts + i;

		// relative velocity at contact
		Vec2 dv = b2->m_velocity + Cross(b2->m_angularVelocity, c->m_r2) - b1->m_velocity - Cross(b1->m_angularVelocity, c->m_r1);

		Vec2 tangent = Cross(c->m_normal, 1.0f);
		float vt = dv.dot(tangent);
		float dJt = c->m_massTangent * (-vt);
		
		// accumulate tangential impulses:
		float maxJt = m_friction * c->m_Jn;
		// clamp friction
		float oldTangentImpulse = c->m_Jt;
		c->m_Jt = std::max(-maxJt, std::min(oldTangentImpulse + dJt, maxJt));
		dJt = c->m_Jt - oldTangentImpulse;

		// apply contact tangent impulse
		Vec2 Jt = tangent * dJt;
		
		tangentialImpulses.push_back(Jt);

		//ESM_LOG("tangent", tangent, "tangential impulse", Jt);
	}

	for (int i = 0; i < m_numContacts; i++)
	{
		Contact* c = m_contacts + i;
		Vec2 Jt = tangentialImpulses[i];
		b1->m_velocity -= Jt * b1->m_invMass;
		b1->m_angularVelocity -= Cross(c->m_r1, Jt) * b1->m_invI;
		b2->m_velocity += Jt * b2->m_invMass;
		b2->m_angularVelocity += Cross(c->m_r2, Jt) * b2->m_invI;
	}
}

