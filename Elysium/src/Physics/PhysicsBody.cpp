#include "PhysicsBody.h"

#include <iostream>

PhysicsBody::PhysicsBody()
	: m_position(0.0f, 0.0f), m_rotation(0.0f), m_velocity(0.0f, 0.0f), m_angularVelocity(0.0f), m_type(PhysicsBodyType::staticBody),
	m_force(0.0f, 0.0f), m_torque(0.0f), m_friction(0.2f), m_restitution(0.5f), m_restitutionThreshold(0.5f), m_density(1.0f), m_mass(FLT_MAX), m_invMass(0.0f), m_I(FLT_MAX), m_invI(0.0f)
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


PhysicsShape* PhysicsBody::GetShape()
{
	return m_shape;
}

PhysicsShape::Type PhysicsBody::GetShapeType() const
{
	return m_shape->GetType();
}

void PhysicsBody::ComputeAABB()
{
	if (m_shape->GetType() == PhysicsShape::e_circle)
	{
		PhysicsCircleShape* circle = static_cast<PhysicsCircleShape*>(m_shape);
		Vec2 center = m_position + circle->m_p;
		m_aabb.min = Vec2(center.x - circle->m_radius, center.y - circle->m_radius);
		m_aabb.max = Vec2(center.x + circle->m_radius, center.y + circle->m_radius);
	}
	else if (m_shape->GetType() == PhysicsShape::e_polygon)
	{
		PhysicsPolygonShape* poly = static_cast<PhysicsPolygonShape*>(m_shape);
		m_aabb.min = Vec2(FLT_MAX, FLT_MAX);
		m_aabb.max = Vec2(-FLT_MAX, -FLT_MAX);
		for (uint32_t i = 0; i < poly->m_count; ++i)
		{
			// transform vertex to world space
			Vec2 v = poly->m_vertices[i].rotate(m_rotation) + m_position;
			if (v.x < m_aabb.min.x) m_aabb.min.x = v.x;
			if (v.y < m_aabb.min.y) m_aabb.min.y = v.y;
			if (v.x > m_aabb.max.x) m_aabb.max.x = v.x;
			if (v.y > m_aabb.max.y) m_aabb.max.y = v.y;
		}
	}
}

void PhysicsBody::ApplyForceToPoint(const Vec2& f, const Vec2& point)
{
	if (m_type != PhysicsBodyType::dynamicBody)
		return;
	// linear
	m_force += f;
	// angular
	Vec2 r = point - m_position;
	m_torque += Cross(r, f);
	WakeUp();
}

void PhysicsBody::ApplyImpulseToCenter(const Vec2& impulse)
{
	if (m_type != PhysicsBodyType::dynamicBody)
		return;
	m_velocity += impulse * m_invMass;
	WakeUp();
}

void PhysicsBody::ApplyImpulseToPoint(const Vec2& impulse, const Vec2& point)
{
	if (m_type != PhysicsBodyType::dynamicBody)
		return;

	// linear
	m_velocity += impulse * m_invMass;

	// angular
	Vec2 r = point - m_position;
	m_angularVelocity += m_invI * Cross(r, impulse);

	WakeUp();
}
