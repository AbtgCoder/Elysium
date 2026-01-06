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

#if 0
int PhysicsPolygonPolygonCollision(Contact* contacts, PhysicsBody* body1, PhysicsBody* body2)
{
	// Safety checks
	if (!body1 || !body2) return 0;
	if (!body1->GetShape() || !body2->GetShape()) return 0;
	if (body1->GetShape()->GetType() != PhysicsShape::e_polygon) return 0;
	if (body2->GetShape()->GetType() != PhysicsShape::e_polygon) return 0;

	PhysicsPolygonShape* polyA = (PhysicsPolygonShape*)body1->GetShape();
	PhysicsPolygonShape* polyB = (PhysicsPolygonShape*)body2->GetShape();

	const uint32_t countA = polyA->m_count;
	const uint32_t countB = polyB->m_count;
	if (countA < 3 || countB < 3) return 0;

	// Build world-space vertex lists for A and B
	std::vector<Vec2> vertsA; vertsA.reserve(countA);
	std::vector<Vec2> vertsB; vertsB.reserve(countB);
	for (uint32_t i = 0; i < countA; ++i)
		vertsA.push_back(body1->m_position + polyA->m_vertices[i].rotate(body1->m_rotation));
	for (uint32_t i = 0; i < countB; ++i)
		vertsB.push_back(body2->m_position + polyB->m_vertices[i].rotate(body2->m_rotation));

	// Build world-space edge normals for each polygon (per-edge normals)
	std::vector<Vec2> normalsA; normalsA.reserve(countA);
	std::vector<Vec2> normalsB; normalsB.reserve(countB);
	for (uint32_t i = 0; i < countA; ++i)
	{
		Vec2 v0 = vertsA[i];
		Vec2 v1 = vertsA[(i + 1) % countA];
		Vec2 edge = v1 - v0;
		Vec2 n = Vec2(-edge.y, edge.x);
		float len = n.length();
		if (len > 1e-6f) n *= (1.0f / len);
		normalsA.push_back(n);
	}
	for (uint32_t i = 0; i < countB; ++i)
	{
		Vec2 v0 = vertsB[i];
		Vec2 v1 = vertsB[(i + 1) % countB];
		Vec2 edge = v1 - v0;
		Vec2 n = Vec2(-edge.y, edge.x);
		float len = n.length();
		if (len > 1e-6f) n *= (1.0f / len);
		normalsB.push_back(n);
	}

	// SAT: test axes = normalsA U normalsB
	float bestOverlap = FLT_MAX;
	Vec2 bestAxis = { 0.0f, 0.0f };
	bool axisFromA = true;

	auto ProjectInterval = [](const std::vector<Vec2>& verts, const Vec2& axis, float& outMin, float& outMax)
		{
			outMin = outMax = axis.dot(verts[0]);
			for (size_t i = 1; i < verts.size(); ++i)
			{
				float p = axis.dot(verts[i]);
				if (p < outMin) outMin = p;
				if (p > outMax) outMax = p;
			}
		};

	// helper to compute overlap between projections
	auto ComputeOverlap = [&](const Vec2& axis, const std::vector<Vec2>& VA, const std::vector<Vec2>& VB) -> float
		{
			float aMin, aMax, bMin, bMax;
			ProjectInterval(VA, axis, aMin, aMax);
			ProjectInterval(VB, axis, bMin, bMax);
			// overlap amount (if positive = overlap, if negative = gap)
			float overlap = std::min(aMax, bMax) - std::max(aMin, bMin);
			return overlap;
		};

	// test normals of A
	for (uint32_t i = 0; i < normalsA.size(); ++i)
	{
		Vec2 axis = normalsA[i];
		float overlap = ComputeOverlap(axis, vertsA, vertsB);
		if (overlap <= 0.0f)
			return 0; // separating axis -> no collision
		if (overlap < bestOverlap)
		{
			bestOverlap = overlap;
			bestAxis = axis;
			axisFromA = true;
		}
	}

	// test normals of B
	for (uint32_t i = 0; i < normalsB.size(); ++i)
	{
		Vec2 axis = normalsB[i];
		float overlap = ComputeOverlap(axis, vertsA, vertsB);
		if (overlap <= 0.0f)
			return 0; // separating axis -> no collision
		if (overlap < bestOverlap)
		{
			bestOverlap = overlap;
			bestAxis = axis;
			axisFromA = false;
		}
	}

	// bestAxis is the minimum-penetration axis (collision normal candidate)
	Vec2 collisionNormal = bestAxis;
	float penetrationDepth = bestOverlap;

	// Ensure the normal points from A to B (so applying positive impulse moves A away from B along -normal)
	Vec2 centerDir = body2->m_position - body1->m_position;
	if (centerDir.dot(collisionNormal) < 0.0f)
		collisionNormal = collisionNormal * -1.0f;

	// Determine reference and incident polygon (reference = face with normal most parallel to collision normal)
	// Compute dot of each polygon's edge normal with collisionNormal and choose the best
	// Build RefEdge struct
	struct RefEdge { Vec2 v1, v2; size_t i1, i2; Vec2 normal; };

	auto GetRefEdge = [&](const std::vector<Vec2>& verts, const std::vector<Vec2>& norms) -> RefEdge
		{
			float best = -FLT_MAX;
			size_t bestIndex = 0;
			for (size_t i = 0; i < norms.size(); ++i)
			{
				float d = norms[i].dot(collisionNormal);
				if (d > best) { best = d; bestIndex = i; }
			}
			RefEdge r;
			r.i1 = bestIndex;
			r.i2 = (bestIndex + 1) % verts.size();
			r.v1 = verts[r.i1];
			r.v2 = verts[r.i2];
			r.normal = norms[r.i1];
			// ensure ref.normal points outward consistent with collisionNormal
			if (r.normal.dot(collisionNormal) < 0.0f) r.normal = r.normal * -1.0f;
			return r;
		};

	RefEdge refA = GetRefEdge(vertsA, normalsA);
	RefEdge refB = GetRefEdge(vertsB, normalsB);

	// Determine which polygon provides the reference face (the one with maximal |dot|)
	float dotA = fabs(refA.normal.dot(collisionNormal));
	float dotB = fabs(refB.normal.dot(collisionNormal));

	bool flip = false;
	RefEdge ref = refA;
	RefEdge inc = refB;
	const std::vector<Vec2>* incVerts = &vertsB;
	const std::vector<Vec2>* refVerts = &vertsA;

	if (dotB > dotA)
	{
		// use B as reference, A as incident
		flip = true;
		ref = refB;
		inc = refA;
		incVerts = &vertsA;
		refVerts = &vertsB;
	}

	// Build incident edge vertices (two endpoints) and give them provisional ids (incident vertex indices)
	struct ClipVert { Vec2 v; ContactID id; };
	ClipVert incident[2];
	incident[0].v = inc.v1;
	incident[1].v = inc.v2;
	// set incident vertex indices in id.cf.indexA (we'll use these as incident indices)
	// When not flipped, inc comes from B so index belongs to B; when flipped, comes from A
	incident[0].id.cf.indexA = (uint8_t)inc.i1;
	incident[1].id.cf.indexA = (uint8_t)inc.i2;
	incident[0].id.cf.typeA = ContactFeature::e_vertex;
	incident[1].id.cf.typeA = ContactFeature::e_vertex;
	// indexB/typeB are unused here for incident array initialization

	// Local lambda for clipping a segment by a single plane.
	auto ClipSegmentToLine = [&](const ClipVert inVerts[2], ClipVert outVerts[2], const Vec2& normal, float offset, uint8_t refEdgeIndex) -> int
		{
			int outCount = 0;
			float d0 = normal.dot(inVerts[0].v) - offset;
			float d1 = normal.dot(inVerts[1].v) - offset;
			if (d0 >= 0.0f) outVerts[outCount++] = inVerts[0];
			if (d1 >= 0.0f) outVerts[outCount++] = inVerts[1];
			if (d0 * d1 < 0.0f)
			{
				float t = d0 / (d0 - d1);
				Vec2 v = inVerts[0].v + (inVerts[1].v - inVerts[0].v) * t;
				// Compose ContactID: ref edge index is refEdgeIndex, incident vertex index copy from closer input (inVerts[0])
				ContactFeature cf;
				cf.indexA = refEdgeIndex;                     // reference edge index on reference poly
				cf.indexB = inVerts[0].id.cf.indexA;          // incident vertex index (from incident polygon)
				cf.typeA = ContactFeature::e_face;
				cf.typeB = ContactFeature::e_vertex;
				ClipVert cv;
				cv.v = v;
				uint32_t key;
				std::memcpy(&key, &cf, sizeof(uint32_t));
				cv.id.key = key;
				outVerts[outCount++] = cv;
			}
			return outCount;
		};

	// Clip incident edge to the side planes of the reference edge
	ClipVert clip1[2], clip2[2];
	Vec2 refEdgeDir = (ref.v2 - ref.v1);
	float lenRef = refEdgeDir.length();
	if (lenRef < 1e-6f) return 0;
	refEdgeDir *= (1.0f / lenRef);

	// plane 1: normal = refEdgeDir, offset = dot(refEdgeDir, ref.v1)
	int np = ClipSegmentToLine(incident, clip1, refEdgeDir, refEdgeDir.dot(ref.v1), (uint8_t)ref.i1);
	if (np < 1) return 0;
	// plane 2: normal = -refEdgeDir, offset = -dot(refEdgeDir, ref.v2)
	np = ClipSegmentToLine(clip1, clip2, refEdgeDir * -1, -refEdgeDir.dot(ref.v2), (uint8_t)ref.i2);
	if (np < 1) return 0;

	// Now clip2 contains up to 2 points. Build final contacts
	int contactCount = 0;
	for (int i = 0; i < 2 && contactCount < 2; ++i)
	{
		// separation relative to reference face normal (positive if outside)
		Vec2 refFaceNormal = ref.normal;
		float separation = refFaceNormal.dot(clip2[i].v - ref.v1);
		const float k_allowedPenetration = 0.002f;
		if (separation <= k_allowedPenetration)
		{
			// Fill contact
			contacts[contactCount].m_position = clip2[i].v;
			// Contact normal should point from A to B always (collisionNormal computed earlier)
			contacts[contactCount].m_normal = collisionNormal;
			// Use negative penetration convention (penetrationDepth positive -> separation negative)
			contacts[contactCount].m_separation = -penetrationDepth;

			// Build stable ContactFeature for this contact:
			ContactFeature cf;
			if (!flip)
			{
				// ref from A, incident from B
				cf.indexA = (uint8_t)ref.i1;
				cf.indexB = clip2[i].id.cf.indexB ? clip2[i].id.cf.indexB : clip2[i].id.cf.indexA;
				// Above: clip2's id.cf.indexB may not be set; fall back to indexA (incident vertex index)
				cf.typeA = ContactFeature::e_face;
				cf.typeB = ContactFeature::e_vertex;
			}
			else
			{
				// ref from B, incident from A -> swap roles
				cf.indexA = clip2[i].id.cf.indexA;
				cf.indexB = (uint8_t)ref.i1;
				cf.typeA = ContactFeature::e_vertex;
				cf.typeB = ContactFeature::e_face;
			}

			uint32_t key;
			std::memcpy(&key, &cf, sizeof(uint32_t));
			contacts[contactCount].m_id.key = key;

			// Initialize accumulators (UpdateContacts will copy old ones if a match is found)
			contacts[contactCount].m_Jn = 0.0f;
			contacts[contactCount].m_Jt = 0.0f;
			contacts[contactCount].m_Jnb = 0.0f;

			++contactCount;
		}
	}

	return contactCount;
}
#endif

#if 0
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
		axesA.push_back(Vec2(edge.y, -1 * edge.x).normalize());
	}
	Vec2 lastEdge = colliderVerticesA[0] - colliderVerticesA.back();
	axesA.push_back(Vec2(lastEdge.y, -1 * lastEdge.x).normalize());

	std::vector<Vec2> axesB;
	for (size_t i = 0; i < colliderVerticesB.size() - 1; i++)
	{
		Vec2 edge = colliderVerticesB[i + 1] - colliderVerticesB[i];
		axesB.push_back(Vec2(edge.y, -1 * edge.x).normalize());
	}
	lastEdge = colliderVerticesB[0] - colliderVerticesB.back();
	axesB.push_back(Vec2(lastEdge.y, -1 * lastEdge.x).normalize());


	float collisionDepth = FLT_MAX;
	Vec2 collisionNormal;

	for (auto axis : axesA)
	{
		float amin = (float)(axis.dot(colliderVerticesA[0])), amax = amin;
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
		float bmin = (float)(axis.dot(colliderVerticesB[0])), bmax = bmin;
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

		/*if ((amin <= bmax && amin >= bmin) || (bmin <= amax && bmin >= amin))
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
		}*/

		if (amax < bmin || bmax < amin)
			return 0;

		float d = std::min(amax - bmin, bmax - amin);
		if (d < collisionDepth)
		{
			collisionDepth = d;
			collisionNormal = axis;
		}

	}

	for (auto axis : axesB)
	{
		float amin = (float)(axis.dot(colliderVerticesA[0])), amax = amin;
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
		float bmin = (float)(axis.dot(colliderVerticesB[0])), bmax = bmin;
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

		/*if ((amin <= bmax && amin >= bmin) || (bmin <= amax && bmin >= amin))
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
		}*/

		if (amax < bmin || bmax < amin)
			return 0;

		float d = std::min(amax - bmin, bmax - amin);
		if (d < collisionDepth)
		{
			collisionDepth = d;
			collisionNormal = axis;
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

	//int numContacts = 0;
	//Vec2 refEdge = ref->v2 - ref->v1;
	//Vec2 refn(refEdge.y, -refEdge.x);
	//refn = refn.normalize();

	//// Ensure A → B
	//if (refn.dot(collisionNormal) < 0.0f)
	//	refn = refn * -1;

	// From now on, this is THE normal
	//collisionNormal = refn;

	for (int i = 0; i < 2; i++)
	{
		float separation = (float)(refn.dot(clipPoints2[i].v) - refn.dot(ref->v));
		if (separation <= 0.002)
		{
			contacts[numContacts].m_separation =  separation;
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
#endif

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