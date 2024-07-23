#pragma once


#include "Math/Vec2.h"
#include "Math/Mat22.h"

#include "PhysicsBody.h"

class PhysicsHingeJoint
{
public:
	PhysicsHingeJoint()
	: m_body1(0), m_body2(0), m_J(0.0f, 0.0f), m_biasFactor(0.2f), m_softness(0.0f) {}

	void Set(PhysicsBody* b1, PhysicsBody* b2, const Vec2& anchor);

	void PreStep(float inv_dt);
	void ApplyImpulse();

public:
	Mat22 m_invK;
	Vec2 m_localAnchor1, m_localAnchor2;
	Vec2 m_r1, m_r2;
	Vec2 m_bias;
	Vec2 m_J; // accumulated impulse
	PhysicsBody* m_body1;
	PhysicsBody* m_body2;
	float m_biasFactor;
	float m_softness;
};