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
		return (v2 - v1).dot(v);
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

	return new CollisionEdge(v, v, v1, index, index2);


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
		float t = d1 / (d1 - d2);
		vOut[count].v = vIn[0].v + (vIn[1].v - vIn[0].v) * t;
		vOut[count].id.cf.indexA = static_cast<uint8_t>(vertexIndexA);
		vOut[count].id.cf.indexB = vIn[0].id.cf.indexB;
		vOut[count].id.cf.typeA = ContactFeature::e_vertex;
		vOut[count].id.cf.typeB = ContactFeature::e_face;
		count += 1;
	}
	return count;
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

	Vec2 direction = pB - pA;
	if (direction.dot(collisionNormal) < 0.0f)
	{
		collisionNormal = collisionNormal * -1; //  because we want the collision normal to be in a direction such that A & B move away from each other
	}

	CollisionEdge* cf1 = FindCollisionEdge(colliderVerticesA, collisionNormal);
	CollisionEdge* cf2 = FindCollisionEdge(colliderVerticesB, collisionNormal * -1);

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

	// Debug printing
	{
		/*std::cout << ref->v << " Reference Edge: " << ref->v1 << " " << ref->v2 << "\n";
		Vec2 v1, v2;
		v1 = flip ? colliderVerticesB[ref->index1] : colliderVerticesA[ref->index1];
		v2 = flip ? colliderVerticesB[ref->index2] : colliderVerticesA[ref->index2];
		std::cout << ref->v << " Reference Edge by indexes: " << v1 << " " << v2 << "\n";
		std::cout << inc->v << " Incident Edge: " << inc->v1 << " " << inc->v2 << "\n";
		v1 = flip ? colliderVerticesA[inc->index1] : colliderVerticesB[inc->index1];
		v2 = flip ? colliderVerticesA[inc->index2] : colliderVerticesB[inc->index2];
		std::cout << inc->v << " Incident Edge by indexes: " << v1 << " " << v2 << "\n";*/
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
	auto np = ClipLineSegment(clipPoints1, cp, refv, refv.dot(ref->v1), ref->index1);   // clip incident edge by the first vertex of reference edge
	if (np < 2)
		return 0;

	np = ClipLineSegment(clipPoints2, clipPoints1, refv * -1, -1 * refv.dot(ref->v2), ref->index2); // clip incident edge by the second vertex of reference edge
	if (np < 2)
		return 0;


	int numContacts = 0;
	Vec2 refn = (ref->v2 - ref->v1).perpendicular().normalize() * -1;
	for (int i = 0; i < 2; i++)
	{
		float separation = refn.dot(clipPoints2[i].v) - refn.dot(ref->v);
		if (separation <= 0.002)
		{
			contacts[numContacts].m_separation = collisionDepth; // separation;
			contacts[numContacts].m_normal = collisionNormal;
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
			//ESM_LOG("contact details: ", contacts[numContacts].m_separation);
			//ESM_LOG("contact details: ", contacts[numContacts].m_normal);
			//ESM_LOG("contact details: ", contacts[numContacts].m_position);
			//ESM_LOG("contact details: ", (int)contacts[numContacts].m_id.cf.indexA);
			//ESM_LOG("contact details: ", (int)contacts[numContacts].m_id.cf.indexB);
			numContacts += 1;
		}
		
	}
	return numContacts;
}
