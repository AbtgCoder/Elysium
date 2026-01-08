#include "PhysicsPolygonCollision.h"


struct CollisionEdge
{
	Vec2 v;
	Vec2 v1;
	Vec2 v2;
	size_t index1;
	size_t index2;
	CollisionEdge(const Vec2& v, const Vec2& v1, const Vec2& v2, size_t i1, size_t i2)
		: v(v), v1(v1), v2(v2), index1(i1), index2(i2) {}

	float dot(const Vec2& v)
	{
		return (float)((v2 - v1).dot(v));
	}
};

struct CollisionVertex
{
	Vec2 v;
	ContactID id;
};

CollisionEdge* FindCollisionEdge(const std::vector<Vec2>& vertices, const Vec2& normal)
{
	float max = -FLT_MAX;
	size_t index = 0;
	for (size_t i = 0; i < vertices.size(); i++)
	{
		float projection = (float)(normal.dot(vertices[i]));
		if (projection > max)
		{
			max = projection;
			index = i;
		}
	}

#if 0
	Vec2 v = vertices[index];
	Vec2 v1 = vertices[(index + 1) % vertices.size()];
	Vec2 v0;
	size_t index1 = index;
	size_t index2 = (index + 1) % vertices.size();
	size_t index3 = index - 1;
	if (index > 0)
	{
		v0 = vertices[index - 1];
	}
	else
	{
		index3 = vertices.size() - 1;
		v0 = vertices.back();
	}
	Vec2 l = (v - v1).normalize();
	Vec2 r = (v - v0).normalize();

	//return new CollisionEdge(v, v, v1, index, index2);


	// the edge that is most perpendicular to n will have a dot product closer to zero
	if (r.dot(normal) <= l.dot(normal))
	{
		// right edge is better
		return new CollisionEdge(v, v0, v, index3, index);
	}
	else
	{
		return new CollisionEdge(v, v, v1, index, index2);
	}
#endif

	size_t i0 = (index + vertices.size() - 1) % vertices.size();
	size_t i1 = index;
	size_t i2 = (index + 1) % vertices.size();

	Vec2 v = vertices[i1];
	Vec2 vPrev = vertices[i0];
	Vec2 vNext = vertices[i2];

	//Vec2 left = (v - vPrev).normalize();
	//Vec2 right = (vNext - v).normalize();

	//// choose edge most perpendicular to normal
	//if (std::abs(left.dot(normal)) <= std::abs(right.dot(normal)))
	//	return new CollisionEdge(v, vPrev, v, i0, i1);
	//else
	//	return new CollisionEdge(v, v, vNext, i1, i2);

	Vec2 n0 = Vec2((v - vPrev).y, -(v - vPrev).x).normalize();
	Vec2 n1 = Vec2((vNext - v).y, -(vNext - v).x).normalize();

	if (n0.dot(normal) > n1.dot(normal))
		return new CollisionEdge(v, vPrev, v, i0, i1);
	else
		return new CollisionEdge(v, v, vNext, i1, i2);

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
		float t = (float)(d1 / (d1 - d2));
		Vec2 pointOnLine = lineStart + (lineEnd - lineStart) * t;
		cp.push_back(pointOnLine);
	}
	return cp;
}

// Sutherland-Hodgman clipping
int32_t ClipLineSegment(CollisionVertex vOut[2], const CollisionVertex vIn[2], const Vec2& normal, float offset, size_t vertexIndexA)
{
	int32_t count = 0;

	double d1 = normal.dot(vIn[0].v) - offset;
	double d2 = normal.dot(vIn[1].v) - offset;
	if (d1 >= 0.0)
		vOut[count++] = vIn[0];
	if (d2 >= 0.0)
		vOut[count++] = vIn[1];

	if (d1 * d2 < 0)
	{
		float t = (float)(d1 / (d1 - d2));
		vOut[count].v = vIn[0].v + (vIn[1].v - vIn[0].v) * t;
		vOut[count].id.cf.indexA = static_cast<uint8_t>(vertexIndexA);
		vOut[count].id.cf.indexB = vIn[0].id.cf.indexB;
		vOut[count].id.cf.typeA = ContactFeature::e_vertex;
		vOut[count].id.cf.typeB = ContactFeature::e_face;
		count += 1;
	}
	return count;
}

inline void ProjectPolygon(const std::vector<Vec2>& verts, const Vec2& axis, float& min, float& max)
{
	min = max = axis.dot(verts[0]);
	for (size_t i = 1; i < verts.size(); ++i)
	{
		float d = axis.dot(verts[i]);
		min = std::min(min, d);
		max = std::max(max, d);
	}
}

int PhysicsPolygonPolygonCollision(Contact* contacts, PhysicsBody* bodyA, PhysicsBody* bodyB)
{
	auto* polyA = (PhysicsPolygonShape*)bodyA->GetShape();
	auto* polyB = (PhysicsPolygonShape*)bodyB->GetShape();

	std::vector<Vec2> vertsA(polyA->m_count);
	std::vector<Vec2> vertsB(polyB->m_count);

	// transform vertices into world space
	for (int i = 0; i < polyA->m_count; ++i)
	{
		vertsA[i] = bodyA->m_position + polyA->m_vertices[i].rotate(bodyA->m_rotation);
	}
	for (int i = 0; i < polyB->m_count; ++i)
	{
		vertsB[i] = bodyB->m_position + polyB->m_vertices[i].rotate(bodyB->m_rotation);
	}

	float minOverlap = FLT_MAX;
	Vec2 bestAxis;

	auto TestAxes = [&](PhysicsPolygonShape* poly, const std::vector<Vec2>& verts, float rotation)
		{
			for (int i = 0; i < poly->m_count; ++i)
			{
				Vec2 axis = poly->m_normals[i].rotate(rotation);

				float minA, maxA, minB, maxB;
				ProjectPolygon(vertsA, axis, minA, maxA);
				ProjectPolygon(vertsB, axis, minB, maxB);

				if (maxA < minB || maxB < minA)
					return false;

				float overlap = std::min(maxA - minB, maxB - minA);
				if (overlap < minOverlap)
				{
					minOverlap = overlap;
					bestAxis = axis;
				}
			}

			return true;
		};

	if (!TestAxes(polyA, vertsA, bodyA->m_rotation)) return 0;
	if (!TestAxes(polyB, vertsB, bodyB->m_rotation)) return 0;

	// Ensure normal points A->B
	Vec2 d = bodyB->m_position - bodyA->m_position;
	if (d.dot(bestAxis) < 0.0f)
		bestAxis = bestAxis * -1.0f;

	CollisionEdge* cf1 = FindCollisionEdge(vertsA, bestAxis);
	CollisionEdge* cf2 = FindCollisionEdge(vertsB, bestAxis * -1);

	// find reference and incident edges (ref edge is the edge most perpendicular to the separation normal)
	CollisionEdge* ref;
	CollisionEdge* inc;
	bool flip = false;
	if (std::abs(cf1->dot(bestAxis)) <= std::abs(cf2->dot(bestAxis)))
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


	CollisionVertex cp[2];
	cp[0].v = inc->v1;
	cp[0].id.cf.indexA = (uint8_t)ref->index1; // use this as edge index
	cp[0].id.cf.indexB = (uint8_t)inc->index1;
	cp[0].id.cf.typeA = ContactFeature::e_face;
	cp[0].id.cf.typeB = ContactFeature::e_vertex;

	cp[1].v = inc->v2;
	cp[1].id.cf.indexA = (uint8_t)ref->index1; // use this as edge index
	cp[1].id.cf.indexB = (uint8_t)inc->index2;
	cp[1].id.cf.typeA = ContactFeature::e_face;
	cp[1].id.cf.typeB = ContactFeature::e_vertex;


	CollisionVertex clipPoints1[2];
	CollisionVertex clipPoints2[2];

	Vec2 refv = (ref->v2 - ref->v1).normalize();
	auto np = ClipLineSegment(clipPoints1, cp, refv, (float)(refv.dot(ref->v1)), ref->index1);   // clip incident edge by the first vertex of reference edge
	if (np < 2)
		return 0;

	np = ClipLineSegment(clipPoints2, clipPoints1, refv * -1, -1 * (float)(refv.dot(ref->v2)), ref->index2); // clip incident edge by the second vertex of reference edge
	if (np < 2)
		return 0;



	int numContacts = 0;
	Vec2 refn = (ref->v2 - ref->v1).perpendicular().normalize() * -1;
	
	for (int i = 0; i < 2; i++)
	{
		float separation = (float)(refn.dot(clipPoints2[i].v) - refn.dot(ref->v));
		if (separation <= 0.002)
		{
			contacts[numContacts].m_separation = separation;
			contacts[numContacts].m_normal = bestAxis;
			contacts[numContacts].m_position = clipPoints2[i].v;
			contacts[numContacts].m_id = clipPoints2[i].id;
			if (flip)
			{
				ContactFeature cf = contacts[numContacts].m_id.cf;
				contacts[numContacts].m_id.cf.indexA = cf.indexB;
				contacts[numContacts].m_id.cf.indexB = cf.indexA;
				contacts[numContacts].m_id.cf.typeA = cf.typeB;
				contacts[numContacts].m_id.cf.typeB = cf.typeA;
			}
			
			numContacts += 1;
		}

	}
	return numContacts;
}