#include "Physics.h"
#include <cmath>
#define INFINITE            0xFFFFFFFF

#if 0

void Physics::NarrowPhaseCollision(KDTreeNode* node)
{
	if (node->left || node->right)
	{
		NarrowPhaseCollision(node->left);
		NarrowPhaseCollision(node->right);
	}
	else
	{
		// must be leaf node, right ? RIGHT ??
		size_t numEntities = node->entities.size();
		if (numEntities < 2)
		{
			return;
		}
		for (size_t i = 0; i < numEntities; i++)
		{
			for (size_t j = i + 1; j < numEntities; j++)
			{
				if (node->entities[i].hasComponent<CPolygonCollider>() && node->entities[j].hasComponent<CPolygonCollider>())
				{
					/*if (Physics::SAT(node->entities[i], node->entities[j]))
					{
						std::cout << node->entities[i].getComponent<CTag>().tag << " collided with " << node->entities[j].getComponent<CTag>().tag << "\n";
					}*/
				}
				else if (node->entities[i].hasComponent<CCircleCollider>() && node->entities[j].hasComponent<CCircleCollider>())
				{
					if (Physics::CircleCircleCollision(node->entities[i], node->entities[j]))
					{
						std::cout << node->entities[i].getComponent<CTag>().tag << " collided with " << node->entities[j].getComponent<CTag>().tag << "\n";
					}
				}
			}
		}
	}
}

Vec2 Physics::GetOverlap(Entity a, Entity b)
{
    Vec2 halfSizeA = a.getComponent<CBoundingBox>().halfSize;
    Vec2 halfSizeB = b.getComponent<CBoundingBox>().halfSize;
    Vec2 offsetA = a.getComponent<CBoundingBox>().offset;
    Vec2 offsetB = b.getComponent<CBoundingBox>().offset;

	Vec2 delta = {0.0f, 0.0f}; // = a.getComponent<CTransform>().GlobalTranslation - b.getComponent<CTransform>().GlobalTranslation + (offsetA - offsetB);
    float ox = halfSizeA.x + halfSizeB.x - abs(delta.x);
    float oy = halfSizeA.y + halfSizeB.y - abs(delta.y);

    return Vec2(ox, oy);
}

Vec2 Physics::GetPreviousOverlap(Entity a, Entity b)
{
    Vec2 halfSizeA = a.getComponent<CBoundingBox>().halfSize;
    Vec2 halfSizeB = b.getComponent<CBoundingBox>().halfSize;
    Vec2 offsetA = a.getComponent<CBoundingBox>().offset;
    Vec2 offsetB = b.getComponent<CBoundingBox>().offset;

   /* Vec2 delta = a.getComponent<CTransform>().prevPos - b.getComponent<CTransform>().prevPos + (offsetA - offsetB);
    float ox = halfSizeA.x + halfSizeB.x - abs(delta.x);
    float oy = halfSizeA.y + halfSizeB.y - abs(delta.y);
    return Vec2(ox, oy);*/
	
	return Vec2(0, 0);
}

std::vector<Vec2> getRotatedRectangleVertices(float centerX, float centerY, float width, float height, float angle)
{
	std::vector<Vec2> vertices;

	// Half dimensions
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	// Convert angle to radians
	float rad = -1 * angle * (3.1412 / 180.0f);

	// Calculate sin and cos of the angle
	float cosA = std::cos(rad);
	float sinA = std::sin(rad);

	// Unrotated vertices relative to the center
	Vec2 relativeVertices[4] = {
		{-halfWidth, -halfHeight},
		{ halfWidth, -halfHeight},
		{ halfWidth,  halfHeight},
		{-halfWidth,  halfHeight}
	};

	// Rotate and translate vertices
	for (int i = 0; i < 4; ++i)
	{
		float x = relativeVertices[i].x;
		float y = relativeVertices[i].y;
		vertices.push_back(Vec2(
			centerX + (x * cosA - y * sinA),
			centerY + (x * sinA + y * cosA)
		));
	}

	return vertices;
}

struct CollisionFeature
{
	Vec2 v;
	Vec2 v1;
	Vec2 v2;

	CollisionFeature(const Vec2& v, const Vec2& v1, const Vec2& v2)
		: v(v), v1(v1), v2(v2) {}

	float dot(const Vec2& v)
	{
		return (v2 - v1).dot(v);
	}
};

CollisionFeature* FindCollisionFeature(const std::vector<Vec2>& vertices, const Vec2& normal)
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
		return new CollisionFeature(v, v0, v);
	}
	else
	{
		return new CollisionFeature(v, v, v1);
	}
}

// Sutherland-Hodgman clipping
std::vector<Vec2> ClipLine(const Vec2& lineStart, const Vec2& lineEnd, const Vec2& normal, float offset)
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

std::vector<Vec2> Physics::AABBCollision(Entity a, Entity b)
{
	std::vector<Vec2> empty;

#if 0

	std::vector<Vec2> colliderVerticesA = getRotatedRectangleVertices(a.getComponent<CTransform>().GlobalTranslation.x, a.getComponent<CTransform>().GlobalTranslation.y, a.getComponent<CBoundingBox>().size.x, a.getComponent<CBoundingBox>().size.y, a.getComponent<CTransform>().GlobalRotation);
	std::vector<Vec2> colliderVerticesB = getRotatedRectangleVertices(b.getComponent<CTransform>().GlobalTranslation.x, b.getComponent<CTransform>().GlobalTranslation.y, b.getComponent<CBoundingBox>().size.x, b.getComponent<CBoundingBox>().size.y, b.getComponent<CTransform>().GlobalRotation);

	// collision normal: axis along which min. penetration occurs
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
			return empty;
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
			return empty;
		}
	}

	Vec2 direction = b.getComponent<CTransform>().GlobalTranslation - a.getComponent<CTransform>().GlobalTranslation;
	if (direction.dot(collisionNormal) < 0.0f)
	{
		collisionNormal = collisionNormal * -1; //  because we want the collision normal to be in a direction such that A & B move away from each other
	}

	CollisionFeature* cf1 = FindCollisionFeature(colliderVerticesA, collisionNormal);
	CollisionFeature* cf2 = FindCollisionFeature(colliderVerticesB, collisionNormal * -1);

	// find reference and incident edges (ref edge is the edge most perpendicular to the separation normal)
	CollisionFeature* ref;
	CollisionFeature* inc;
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

	std::cout << ref->v << " Reference face: " << ref->v1 << " " << ref->v2 << "\n";
	std::cout << inc->v << " Incident face: "  << inc->v1 << " " << inc->v2 << "\n";
	Vec2 refv = (ref->v2 - ref->v1).normalize();
	auto cp = ClipLine(inc->v1, inc->v2, refv, refv.dot(ref->v1));   // clip incident edge by the first vertex of reference edge
	if (cp.size() < 2)
		return empty;

	cp = ClipLine(cp[0], cp[1], refv * -1, -1 * refv.dot(ref->v2)); // clip incident edge by the second vertex of reference edge
	if (cp.size() < 2)
		return empty;


	// get the reference edge normal
	Vec2 refn = (ref->v2 - ref->v1).perpendicular().normalize();
	// if we had to flip the incident and reference edges
	// then we need to flip the reference edge normal to
	// clip properly
	//TODO: STUFF SEEMS TO WORK WITHOUT USING THIS FLIP THING
	/*if (flip)
	{
		refn = refn * -1; 
	}*/
	double max = refn.dot(ref->v);
	// make sure the final points are not past this maximum
	if (refn.dot(cp[0]) - max < 0.0)
	{
		cp.erase(cp.begin()); 
	}
	if (refn.dot(cp[1]) - max < 0.0)
	{
		cp.erase(cp.begin()+1);
	}

	return cp;

	Vec2 overlap = GetOverlap(a, b);
	if (overlap.x > 0 && overlap.y > 0)
	{
		// horizontal
		float v1y = a.getComponent<CTransform>().velocity.y;
		float v1x = a.getComponent<CTransform>().velocity.x;
		float v2y = b.getComponent<CTransform>().velocity.y;
		float v2x = b.getComponent<CTransform>().velocity.x;

		// collision
		float m1 = 1.0f;
		float e1 = 1.0f;
		float m2 = 1.0f;
		float e2 = 1.0f;

		if (a.hasComponent<CPhysicsMaterial>())
		{
			m1 = a.getComponent<CPhysicsMaterial>().mass;
			e1 = a.getComponent<CPhysicsMaterial>().restitutionCoefficient;
		}
		if (b.hasComponent<CPhysicsMaterial>())
		{
			m2 = b.getComponent<CPhysicsMaterial>().mass;
			e2 = b.getComponent<CPhysicsMaterial>().restitutionCoefficient;
		}

		Vec2 prevOverlap = GetPreviousOverlap(a, b);
		if (prevOverlap.y > 0)
		{
			// horizontal
			a.getComponent<CTransform>().velocity.x = (v1x * (m1 - m2) + ((1 + e1) * m2 * v2x)) / (m1 + m2);
			b.getComponent<CTransform>().velocity.x = (v2x * (m2 - m1) + ((1 + e2) * m1 * v1x)) / (m1 + m2);
		}
		if (prevOverlap.x > 0)
		{
			// vertical
			a.getComponent<CTransform>().velocity.y = (v1y * (m1 - m2) + ((1 + e1) * m2 * v2y)) / (m1 + m2);
			b.getComponent<CTransform>().velocity.y = (v2y * (m2 - m1) + ((1 + e2) * m1 * v1y)) / (m1 + m2);
		}
		return true;
	}
	return false;
#endif

	return empty;
}

bool Physics::CircleCircleCollision(Entity a, Entity b)
{
#if 0
	Vec2 c1 = a.getComponent<CTransform>().pos;
	Vec2 c2 = b.getComponent<CTransform>().pos;
	float r1 = a.getComponent<CCircleCollider>().radius;
	float r2 = b.getComponent<CCircleCollider>().radius;

	Vec2 c1_c2 = c1 - c2;
	float len_c1_c2 = c1_c2.length();
	if (len_c1_c2 <= r1 + r2)
	{
		// collision
		float m1 = 1.0f;
		float e1 = 1.0f;
		float m2 = 1.0f;
		float e2 = 1.0f;
		float f1 = 0.2f;
		float f2 = 0.2f;

		if (a.hasComponent<CPhysicsMaterial>())
		{
			m1 = a.getComponent<CPhysicsMaterial>().mass;
			e1 = a.getComponent<CPhysicsMaterial>().restitutionCoefficient;
			f1 = a.getComponent<CPhysicsMaterial>().friction;
		}
		if (b.hasComponent<CPhysicsMaterial>())
		{
			m2 = b.getComponent<CPhysicsMaterial>().mass;
			e2 = b.getComponent<CPhysicsMaterial>().restitutionCoefficient;
			f2 = b.getComponent<CPhysicsMaterial>().friction;
		}
		float e = std::min(e1, e2);
		float friction = sqrtf(f1 * f2);
		// normal impulse
		Vec2 v1_v2 = a.getComponent<CTransform>().velocity - b.getComponent<CTransform>().velocity;
		float v1_v2_dot_c1_c2 = v1_v2.x * c1_c2.x + v1_v2.y * c1_c2.y;
		float dJn = ((v1_v2_dot_c1_c2) / (len_c1_c2 * len_c1_c2)) * ((1 + e) / (1/m1 + 1/m2));
		Vec2 Jn = c1_c2 * dJn;
		std::cout << "normal: " << c1_c2.normalize() <<  " dv: " << v1_v2 << " dJn: " << dJn << " impulse: " << Jn << "\n";
		//ESM_LOG("old velocities", a.getComponent<CTransform>().velocity, b.getComponent<CTransform>().velocity);
		a.getComponent<CTransform>().velocity = a.getComponent<CTransform>().velocity - Jn / m1;
		b.getComponent<CTransform>().velocity = b.getComponent<CTransform>().velocity + Jn / m2;
		//ESM_LOG("new velocities", a.getComponent<CTransform>().velocity, b.getComponent<CTransform>().velocity);

		// tangential impulse (due to friction)
		Vec2 tangent = { c1_c2.y, -1 * c1_c2.x };
		float vt = v1_v2.dot(tangent);
		float dJt = vt * ((1 + e) * m1 * m2 / (m1 + m2));
		dJt = std::max(-1 * friction * dJn, std::min(dJt, friction * dJn));
		Vec2 Jt = tangent * dJt;
		a.getComponent<CTransform>().velocity = a.getComponent<CTransform>().velocity - Jt / m1;
		b.getComponent<CTransform>().velocity = b.getComponent<CTransform>().velocity + Jt / m2;
		return true;
	}
	else
	{
		return false;
	}
#endif

	return false;
}

std::vector<Vec2> Physics::SAT(Entity a, Entity b)
{
	std::vector<Vec2> empty;
#if 0

	std::vector<Vec2> colliderVerticesA;
	Vec2 aPos = a.getComponent<CTransform>().pos;
	Vec2 aSize = a.getComponent<CPolygonCollider>().size;

	std::vector<Vec2> convexHullA = a.getComponent<CPolygonCollider>().colliderVertices;
	for (auto p : convexHullA)
	{
		colliderVerticesA.push_back(aPos - (aPos - Vec2(aPos.x - aSize.x / 2 + p.x, aPos.y + aSize.y / 2 - p.y)).rotate(a.getComponent<CTransform>().angle));
	}
	std::vector<Vec2> colliderVerticesB;
	Vec2 bPos = b.getComponent<CTransform>().pos;
	Vec2 bSize = b.getComponent<CPolygonCollider>().size;
	std::vector<Vec2> convexHullB = b.getComponent<CPolygonCollider>().colliderVertices;
	for (auto p : convexHullB)
	{
		colliderVerticesB.push_back(bPos - (bPos - Vec2(bPos.x - bSize.x / 2 + p.x, bPos.y + bSize.y / 2 - p.y)).rotate(b.getComponent<CTransform>().angle));
	}

	// collision normal: axis along which min. penetration occurs
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
			return empty;
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
			return empty;
		}
	}

	Vec2 direction = b.getComponent<CTransform>().pos - a.getComponent<CTransform>().pos;
	if (direction.dot(collisionNormal) < 0.0f)
	{
		collisionNormal = collisionNormal * -1; //  because we want the collision normal to be in a direction such that A & B move away from each other
	}

	CollisionFeature* cf1 = FindCollisionFeature(colliderVerticesA, collisionNormal);
	CollisionFeature* cf2 = FindCollisionFeature(colliderVerticesB, collisionNormal * -1);

	// find reference and incident edges (ref edge is the edge most perpendicular to the separation normal)
	CollisionFeature* ref;
	CollisionFeature* inc;
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

	std::cout << ref->v << " Reference face: " << ref->v1 << " " << ref->v2 << "\n";
	std::cout << inc->v << " Incident face: " << inc->v1 << " " << inc->v2 << "\n";
	Vec2 refv = (ref->v2 - ref->v1).normalize();
	auto cp = ClipLine(inc->v1, inc->v2, refv, refv.dot(ref->v1));   // clip incident edge by the first vertex of reference edge
	if (cp.size() < 2)
		return empty;

	cp = ClipLine(cp[0], cp[1], refv * -1, -1 * refv.dot(ref->v2)); // clip incident edge by the second vertex of reference edge
	if (cp.size() < 2)
		return empty;

	return cp;

	// get the reference edge normal
	Vec2 refn = (ref->v2 - ref->v1).perpendicular().normalize();
	// if we had to flip the incident and reference edges
	// then we need to flip the reference edge normal to
	// clip properly
	if (flip)
	{
		refn = refn * -1;
	}
	double max = refn.dot(ref->v);
	//ESM_LOG("flip", flip, "refn", refn, "vmax", max);
	// make sure the final points are not past this maximum
	if (refn.dot(cp[0]) - max < 0.0)
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
	}

	//cp.push_back(ref->v1);
	//cp.push_back(ref->v2);


	return cp;

#endif

	return empty;
}


#endif