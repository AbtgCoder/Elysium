#include "PhysicsBody.h"

#include <iostream>

PhysicsBody::PhysicsBody()
	: m_position(0.0f, 0.0f), m_rotation(0.0f), m_velocity(0.0f, 0.0f), m_angularVelocity(0.0f), m_type(PhysicsBodyType::staticBody),
	  m_force(0.0f, 0.0f), m_torque(0.0f), m_friction(0.2f), m_density(1.0f), m_mass(FLT_MAX), m_invMass(0.0f), m_I(FLT_MAX), m_invI(0.0f)
{

}

void PhysicsBody::ResetMassData(float density)
{
	m_density = density;
	m_mass = 0.0f;
	m_invMass = 0.0f;
	m_I = 0.0f;
	m_invI = 0.0f;

	// static & kinematic body have zero mass ??
	if (m_type == PhysicsBodyType::staticBody)
	{
		/*m_mass = FLT_MAX;
		m_I = FLT_MAX;*/
		return;
	}

	// get mass data (TODO: normally this would be accumulated over all fixtures...)
	Vec2 localCenter;
	{
		PhysicsMassData massData;
		m_shape->ComputeMass(&massData, m_density);
		m_mass = massData.mass;
		localCenter = massData.center * massData.mass;
		m_I = massData.I;
	}
	// compute center of mass
	if (m_mass > 0.0f)
	{
		m_invMass = 1.0f / m_mass;
		localCenter = localCenter * m_invMass;
	}

	if (m_I > 0.0f)
	{
		// center inertia about center of mass
		m_I -= m_mass * localCenter.dot(localCenter);
		// assert m_I > 0.0f
		m_invI = 1.0f / m_I;
	}
	else
	{
		m_I = 0.0f;
		m_invI = 0.0f;
	}

}

void PhysicsBody::AddForce(const Vec2& f)
{
	m_force += f;
}

PhysicsShape* PhysicsBody::GetShape()
{
	return m_shape;
}

PhysicsShape::Type PhysicsBody::GetShapeType() const
{
	return m_shape->GetType();
}
