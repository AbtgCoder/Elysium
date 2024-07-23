#include "PhysicsHingeJoint.h"

#include "core/Log.h"

// Point-to-point constraint
// C = p2 - p1
// Cdot = v2 - v1
//      = v2 + cross(w2, r2) - v1 - cross(w1, r1)
// J = [-I -r1_skew I r2_skew ]
// r_skew = [-ry; rx]
// Identity used:
// w k % (rx i + ry j) = w * (-ry i + rx j)

void PhysicsHingeJoint::Set(PhysicsBody* b1, PhysicsBody* b2, const Vec2& anchor)
{
	m_body1 = b1;
	m_body2 = b2;

	Mat22 Rot1(m_body1->m_rotation);
	Mat22 Rot2(m_body2->m_rotation);
	Mat22 Rot1T = Rot1.Transpose();
	Mat22 Rot2T = Rot2.Transpose();

	m_localAnchor1 = Rot1T * (anchor - m_body1->m_position);
	m_localAnchor2 = Rot2T * (anchor - m_body2->m_position);

	m_J.Set(0.0f, 0.0f);
	m_softness = 0.0f;
	m_biasFactor = 0.2f;
}

void PhysicsHingeJoint::PreStep(float inv_dt)
{
	// pre compute anchors, K = J * M-1 * Jt( J transpose), and bias
	Mat22 Rot1(m_body1->m_rotation);
	Mat22 Rot2(m_body2->m_rotation);

	m_r1 = Rot1 * m_localAnchor1;
	m_r2 = Rot2 * m_localAnchor2;

	// deltaV = deltaV0 + K * impulse
	// invM = [(1/m1 + 1/m2) * eye(2) - skew(r1) * invI1 * skew(r1) - skew(r2) * invI2 * skew(r2)]
	//      = [1/m1+1/m2     0    ] + invI1 * [r1.y*r1.y -r1.x*r1.y] + invI2 * [r1.y*r1.y -r1.x*r1.y]
	//        [    0     1/m1+1/m2]           [-r1.x*r1.y r1.x*r1.x]           [-r1.x*r1.y r1.x*r1.x]
	Mat22 K1;
	K1.col1.x = m_body1->m_invMass + m_body2->m_invMass;	K1.col2.x = 0.0f;
	K1.col1.y = 0.0f;										K1.col2.y = m_body1->m_invMass + m_body2->m_invMass;

	Mat22 K2;
	K2.col1.x =  m_body1->m_invI * m_r1.y * m_r1.y;	K2.col2.x = -m_body1->m_invI * m_r1.x * m_r1.y;
	K2.col1.y = -m_body1->m_invI * m_r1.x * m_r1.y;	K2.col2.x =  m_body1->m_invI * m_r1.x * m_r1.x;

	Mat22 K3;
	K3.col1.x =  m_body2->m_invI * m_r2.y * m_r2.y; K3.col2.x = -m_body2->m_invI * m_r2.x * m_r2.y;
	K3.col1.y = -m_body2->m_invI * m_r2.x * m_r2.y;	K3.col2.y =  m_body2->m_invI * m_r2.x * m_r2.x;

	Mat22 K = K1 + K2 + K3;
	K.col1.x += m_softness;
	K.col2.y += m_softness;

	m_invK = K.Invert();

	Vec2 p1 = m_body1->m_position + m_r1;
	Vec2 p2 = m_body2->m_position + m_r2;
	Vec2 dp = p2 - p1; // for position constraint this should be the same
	// position correction:
	m_bias = dp * -1 * m_biasFactor * inv_dt;

	// warm starting:
	m_body1->m_velocity -= m_J * m_body1->m_invMass;
	m_body1->m_angularVelocity -= Cross(m_r1, m_J) * m_body1->m_invMass;
	m_body2->m_velocity += m_J * m_body2->m_invMass;
	m_body2->m_angularVelocity += Cross(m_r2, m_J) * m_body2->m_invMass;

}

void PhysicsHingeJoint::ApplyImpulse()
{
	Vec2 dv = m_body2->m_velocity + Cross(m_body2->m_angularVelocity, m_r2) - m_body1->m_velocity - Cross(m_body1->m_angularVelocity, m_r1);

	Vec2 impulse = m_invK * (m_bias - dv - m_J * m_softness);
	
	m_body1->m_velocity -= impulse * m_body1->m_invMass;
	m_body1->m_angularVelocity -= Cross(m_r1, impulse) * m_body1->m_invMass;
	m_body2->m_velocity += impulse * m_body2->m_invMass;
	m_body2->m_angularVelocity += Cross(m_r2, impulse) * m_body2->m_invMass;

	m_J += impulse;

}
