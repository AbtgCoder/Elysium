#include "PhysicsCircleShape.h"

void PhysicsCircleShape::ComputeMass(PhysicsMassData* massData, float density) const
{
	massData->mass = density * 3.14 * m_radius * m_radius;
	massData->center = m_p;

	// inertia about local origin
	massData->I = massData->mass * (0.5f * m_radius * m_radius + m_p.dot(m_p));
}