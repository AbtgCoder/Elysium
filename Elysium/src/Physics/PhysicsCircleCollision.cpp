#include "PhysicsCircleCollision.h"

#include "core/Log.h"

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

int PhysicsCirclePolygonCollision(Contact* contacts, PhysicsBody* body1, PhysicsBody* body2)
{
	PhysicsCircleShape* circle;
	PhysicsPolygonShape* poly;
	bool flip = false;
	if (body1->GetShapeType() == PhysicsShape::Type::e_circle)
	{
		circle = (PhysicsCircleShape*)body1->GetShape();
		poly = (PhysicsPolygonShape*)body2->GetShape();
	}
	else
	{
		flip = true;
		circle = (PhysicsCircleShape*)body2->GetShape();
		poly = (PhysicsPolygonShape*)body1->GetShape();
	}

	Vec2 circlePos;
	float circleRadius = circle->m_radius;
	Vec2 polyPos;
	float polyRotation;
	int numPolyVertices = poly->m_count;
	const Vec2* polyVertices = poly->m_vertices;

	if (flip)
	{
		circlePos = body2->m_position;
		polyPos = body1->m_position;
		polyRotation = body1->m_rotation;
	}
	else
	{
		circlePos = body1->m_position;
		polyPos = body2->m_position;
		polyRotation = body2->m_rotation;
	}

	std::vector<Vec2> polyColliderVertices;
	for (int i = 0; i < numPolyVertices; i++)
	{
		polyColliderVertices.push_back(polyPos + polyVertices[i].rotate(polyRotation));
	}

	std::vector<Vec2> axesToTest;
	for (size_t i = 0; i < polyColliderVertices.size() - 1; i++)
	{
		Vec2 edge = polyColliderVertices[i + 1] - polyColliderVertices[i];
		axesToTest.push_back(Vec2(-1 * edge.y, edge.x).normalize());
	}
	Vec2 lastEdge = polyColliderVertices[0] - polyColliderVertices.back();
	axesToTest.push_back(Vec2(-1 * lastEdge.y, lastEdge.x).normalize());

	Vec2 closestVertex;
	float closestVertexDist = FLT_MAX;
	for (auto v : polyColliderVertices)
	{
		float d = (circlePos - v).dot(circlePos - v);
		if (d < closestVertexDist)
		{
			closestVertexDist = d;
			closestVertex = v;
		}
	}
	axesToTest.push_back((closestVertex - circlePos).normalize());

	// test for overlap
	float collisionDepth = FLT_MAX;
	Vec2 collisionNormal;
	for (auto axis : axesToTest)
	{
		float amin = axis.dot(polyColliderVertices[0]), amax = amin;
		for (size_t i = 0; i < polyColliderVertices.size(); i++)
		{
			Vec2 p = polyColliderVertices[i];
			float dot = axis.x * p.x + axis.y * p.y;
			if (dot > amax)
			{
				amax = dot;
			}
			else if (dot < amin)
			{
				amin = dot;
			}
		}
		
		// circle proj
		Vec2 p1 = circlePos - axis * circleRadius;
		Vec2 p2 = circlePos + axis * circleRadius;
		float dot1 = axis.dot(p1);
		float dot2 = axis.dot(p2);
		float bmin = dot1, bmax = dot2;
		if (dot2 < dot1)
		{
			bmin = dot2;
			bmax = dot1;
		}

		if ((amin <= bmax && amin >= bmin) || (bmin <= amax && bmin >= amin))
		{
			float d = std::min(bmax - amin, amax - bmin);
			if (d < collisionDepth)
			{
				collisionDepth = d;
				collisionNormal = axis;
			}
		}
		else
		{
			return 0;
		}
	}

	Vec2 direction = flip ? polyPos - circlePos : circlePos - polyPos;
	if (direction.dot(collisionNormal) < 0.0f)
	{
		collisionNormal = collisionNormal * -1; 
	}
	ESM_LOG("collision normal", collisionNormal, "collision depth", collisionDepth);

	contacts[0].m_separation = collisionDepth;
	contacts[0].m_normal = collisionNormal;
	contacts[0].m_position = flip ? circlePos + collisionNormal * circleRadius : circlePos - collisionNormal * circleRadius; // TODO: check if this works in every case
	contacts[0].m_id.key = 0;
	return 1;
}
