#include "PhysicsCircleCollision.h"
#include <iostream>
int PhysicsCircleCircleCollision(Contact* contacts, PhysicsBody* body1, PhysicsBody* body2)
{
	PhysicsCircleShape* circleA = (PhysicsCircleShape*)body1->GetShape();
	PhysicsCircleShape* circleB = (PhysicsCircleShape*)body2->GetShape();

	int numContacts = 0;

	Vec2 pA = body1->m_position;
	Vec2 pB = body2->m_position;
	
	Vec2 d = pA - pB;
	float distSqr = d.dot(d);
	float radius = circleA->m_radius + circleB->m_radius;
	if (distSqr > radius * radius)
	{
		return numContacts;
	}

	contacts[numContacts].m_separation = d.length() - radius;
	contacts[numContacts].m_normal = d.normalize();
	contacts[numContacts].m_position = pA - contacts[numContacts].m_normal * circleA->m_radius;
	contacts[numContacts].m_id.key = 0; // does this work or nah
	numContacts += 1;
	return numContacts;
}