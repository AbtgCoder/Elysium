#include "PhysicsPolygonCollision.h"

#include <iostream>

struct CollisionEdge
{
	Vec2 v;
	Vec2 v1;
	Vec2 v2;

	CollisionEdge(const Vec2& v, const Vec2& v1, const Vec2& v2)
		: v(v), v1(v1), v2(v2) {}

	float dot(const Vec2& v)
	{
		return (v2 - v1).dot(v);
	}
};

CollisionEdge* FindCollisionEdge(const std::vector<Vec2>& vertices, const Vec2& normal)
{
	float max = -FLT_MAX;
	size_t index = 0;
	for (size_t i = 0; i < vertices.size(); i++)
	{
		float projection = normal.dot(vertices[i]);
		if (projection > max)
		{
			max = projection;
			index = i;
		}
	}
	Vec2 v = vertices[index];
	Vec2 v1 = vertices[(index + 1) % vertices.size()];
	Vec2 v0;
	if (index > 0)
	{
		v0 = vertices[index - 1];
	}
	else
	{
		v0 = vertices.back();
	}
	Vec2 l = (v - v1).normalize();
	Vec2 r = (v - v0).normalize();
	// the edge that is most perpendicular to n will have a dot product closer to zero
	if (r.dot(normal) <= l.dot(normal))
	{
		// right edge is better
		return new CollisionEdge(v, v0, v);
	}
	else
	{
		return new CollisionEdge(v, v, v1);
	}
}

// Sutherland-Hodgman clipping
std::vector<Vec2> ClipLineSegment(const Vec2& lineStart, const Vec2& lineEnd, const Vec2& normal, float offset)
{
	std::vector<Vec2> cp;
	double d1 = normal.dot(lineStart) - offset;
	double d2 = normal.dot(lineEnd) - offset;
	if (d1 >= 0.0)
		cp.push_back(lineStart);
	if (d2 >= 0.0)
		cp.push_back(lineEnd);

	if (d1 * d2 < 0)
	{
		float t = d1 / (d1 - d2);
		Vec2 pointOnLine = lineStart + (lineEnd - lineStart) * t;
		cp.push_back(pointOnLine);
	}
	return cp;
}

int PhysicsPolygonPolygonCollision(Contact* contacts, PhysicsBody* body1, PhysicsBody* body2)
{
	Vec2 pA = body1->m_position;
	float rA = body1->m_rotation;
	PhysicsPolygonShape* polyA = (PhysicsPolygonShape*)body1->GetShape();
	int countA = polyA->m_count;
	const Vec2* verticesA = polyA->m_vertices;
	Vec2 pB = body2->m_position;
	float rB = body2->m_rotation;
	PhysicsPolygonShape* polyB = (PhysicsPolygonShape*)body2->GetShape();
	int countB = polyB->m_count;
	const Vec2* verticesB = polyB->m_vertices;



	std::vector<Vec2> colliderVerticesA;
	std::vector<Vec2> colliderVerticesB;

	for (int i = 0; i < countA; i++)
	{
		colliderVerticesA.push_back(pA + verticesA[i].rotate(rA));
	}

	for (int i = 0; i < countB; i++)
	{
		colliderVerticesB.push_back(pB + verticesB[i].rotate(rB));
	}


	// TODO: use shape normals
	std::vector<Vec2> axesA;
	for (size_t i = 0; i < colliderVerticesA.size() - 1; i++)
	{
		Vec2 edge = colliderVerticesA[i + 1] - colliderVerticesA[i];
		axesA.push_back(Vec2(-1 * edge.y, edge.x).normalize());
	}
	Vec2 lastEdge = colliderVerticesA[0] - colliderVerticesA.back();
	axesA.push_back(Vec2(-1 * lastEdge.y, lastEdge.x).normalize());

	std::vector<Vec2> axesB;
	for (size_t i = 0; i < colliderVerticesB.size() - 1; i++)
	{
		Vec2 edge = colliderVerticesB[i + 1] - colliderVerticesB[i];
		axesB.push_back(Vec2(-1 * edge.y, edge.x).normalize());
	}
	lastEdge = colliderVerticesB[0] - colliderVerticesB.back();
	axesB.push_back(Vec2(-1 * lastEdge.y, lastEdge.x).normalize());

	float collisionDepth = FLT_MAX;
	Vec2 collisionNormal;

	for (auto axis : axesA)
	{
		float amin = axis.dot(colliderVerticesA[0]), amax = amin;
		for (size_t i = 0; i < colliderVerticesA.size(); i++)
		{
			Vec2 p = colliderVerticesA[i];
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
		float bmin = axis.dot(colliderVerticesB[0]), bmax = bmin;
		for (size_t i = 0; i < colliderVerticesB.size(); i++)
		{
			Vec2 p = colliderVerticesB[i];
			float dot = axis.x * p.x + axis.y * p.y;
			if (dot > bmax)
			{
				bmax = dot;
			}
			else if (dot < bmin)
			{
				bmin = dot;
			}
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

	for (auto axis : axesB)
	{
		float amin = axis.dot(colliderVerticesA[0]), amax = amin;
		for (size_t i = 0; i < colliderVerticesA.size(); i++)
		{
			Vec2 p = colliderVerticesA[i];
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
		float bmin = axis.dot(colliderVerticesB[0]), bmax = bmin;
		for (size_t i = 0; i < colliderVerticesB.size(); i++)
		{
			Vec2 p = colliderVerticesB[i];
			float dot = axis.x * p.x + axis.y * p.y;
			if (dot > bmax)
			{
				bmax = dot;
			}
			else if (dot < bmin)
			{
				bmin = dot;
			}
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

	Vec2 direction = pA - pB;
	if (direction.dot(collisionNormal) < 0.0f)
	{
		collisionNormal = collisionNormal * -1; //  because we want the collision normal to be in a direction such that A & B move away from each other
	}

	CollisionEdge* cf1 = FindCollisionEdge(colliderVerticesA, collisionNormal * -1);
	CollisionEdge* cf2 = FindCollisionEdge(colliderVerticesB, collisionNormal);

	// find reference and incident edges (ref edge is the edge most perpendicular to the separation normal)
	CollisionEdge* ref;
	CollisionEdge* inc;
	bool flip = false;
	if (std::abs(cf1->dot(collisionNormal)) <= std::abs(cf2->dot(collisionNormal)))
	{
		ref = cf1;
		inc = cf2;
	}
	else
	{
		ref = cf2;
		inc = cf1;
		// we need to set a flag indicating that the reference
		// and incident edge were flipped so that when we do the final
		// clip operation, we use the right edge normal
		flip = true;
	}

	//std::cout << ref->v << " Reference Edge: " << ref->v1 << " " << ref->v2 << "\n";
	//std::cout << inc->v << " Incident Edge: " << inc->v1 << " " << inc->v2 << "\n";
	Vec2 refv = (ref->v2 - ref->v1).normalize();
	auto cp = ClipLineSegment(inc->v1, inc->v2, refv, refv.dot(ref->v1));   // clip incident edge by the first vertex of reference edge
	if (cp.size() < 2)
		return 0;

	cp = ClipLineSegment(cp[0], cp[1], refv * -1, -1 * refv.dot(ref->v2)); // clip incident edge by the second vertex of reference edge
	if (cp.size() < 2)
		return 0;

	//return cp;

	// get the reference edge normal
	Vec2 refn = (ref->v2 - ref->v1).perpendicular().normalize();
	// if we had to flip the incident and reference edges
	// then we need to flip the reference edge normal to
	// clip properly
	/*if (flip)
	{
		refn = refn * -1;
	}*/
	double max = refn.dot(ref->v);
	// make sure the final points are not past this maximum
	/*if (refn.dot(cp[0]) - max < 0.0)
	{
		cp.erase(cp.begin());
		if (refn.dot(cp[0]) - max < 0.0)
		{
			cp.erase(cp.begin());
		}

	}
	else if (refn.dot(cp[1]) - max < 0.0)
	{
		cp.erase(cp.begin() + 1);
	}*/

	for (size_t i=0; i < cp.size(); i++)
	{
		contacts[i].m_separation = collisionDepth; //TODO: each contact point may have different depth
		contacts[i].m_normal = collisionNormal;
		contacts[i].m_position = cp[i];
		//TODO: contact id/features of these points
	}

	return cp.size();
}
