#include "Physics.h"
#include <cmath>

#define INFINITE            0xFFFFFFFF

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
					if (Physics::SAT(node->entities[i], node->entities[j]))
					{
						std::cout << node->entities[i].getComponent<CTag>().tag << " collided with " << node->entities[j].getComponent<CTag>().tag << "\n";
					}
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

    Vec2 delta = a.getComponent<CTransform>().pos - b.getComponent<CTransform>().pos + (offsetA - offsetB);
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

    Vec2 delta = a.getComponent<CTransform>().prevPos - b.getComponent<CTransform>().prevPos + (offsetA - offsetB);
    float ox = halfSizeA.x + halfSizeB.x - abs(delta.x);
    float oy = halfSizeA.y + halfSizeB.y - abs(delta.y);

    return Vec2(ox, oy);
}

bool Physics::CircleCircleCollision(Entity a, Entity b)
{
	Vec2 c1 = a.getComponent<CTransform>().pos;
	Vec2 c2 = b.getComponent<CTransform>().pos;
	float r1 = a.getComponent<CCircleCollider>().radius;
	float r2 = b.getComponent<CCircleCollider>().radius;

	Vec2 c1_c2 = c1 - c2;
	float len_c1_c2 = c1_c2.length();
	if (len_c1_c2 <= r1 + r2)
	{
		// elastic collision
		float m1 = 1.0f;
		float m2 = 1.0f;
		if (a.hasComponent<CPhysicsMaterial>())
		{
			m1 = a.getComponent<CPhysicsMaterial>().mass;
		}
		if (b.hasComponent<CPhysicsMaterial>())
		{
			m2 = b.getComponent<CPhysicsMaterial>().mass;
		}
		Vec2 v1_v2 = a.getComponent<CTransform>().velocity - b.getComponent<CTransform>().velocity;
		float v1_v2_dot_c1_c2 = v1_v2.x * c1_c2.x + v1_v2.y * c1_c2.y;
		a.getComponent<CTransform>().velocity = a.getComponent<CTransform>().velocity - (c1 - c2) * ((v1_v2_dot_c1_c2) / (len_c1_c2 * len_c1_c2)) * (2 * m2 / (m1 + m2));
		b.getComponent<CTransform>().velocity = b.getComponent<CTransform>().velocity - (c2 - c1) * ((v1_v2_dot_c1_c2) / (len_c1_c2 * len_c1_c2)) * (2 * m1 / (m1 + m2));
		return true;
	}
	else
	{
		return false;
	}
}



bool Physics::SAT(Entity a, Entity b)
{
	std::vector<Vec2> colliderVerticesA;
	Vec2 aPos = a.getComponent<CTransform>().pos;
	Vec2 aSize = a.getComponent<CPolygonCollider>().size;
	std::vector<Vec2> convexHullA = a.getComponent<CPolygonCollider>().colliderVertices;
	for (auto p : convexHullA)
	{
		colliderVerticesA.push_back(Vec2(aPos.x - aSize.x / 2 + p.x, aPos.y + aSize.y / 2 - p.y));
	}
	std::vector<Vec2> colliderVerticesB;
	Vec2 bPos = b.getComponent<CTransform>().pos;
	Vec2 bSize = b.getComponent<CPolygonCollider>().size;
	std::vector<Vec2> convexHullB = b.getComponent<CPolygonCollider>().colliderVertices;
	for (auto p : convexHullB)
	{
		colliderVerticesB.push_back(Vec2(bPos.x - bSize.x / 2 + p.x, bPos.y + bSize.y / 2 - p.y));
	}

	if ((colliderVerticesA.size() == 0 || colliderVerticesB.size() == 0))
	{
		return false;
	}
	

	std::vector<Vec2> axes;

	for (size_t i = 0; i < colliderVerticesA.size() - 1; i++)
	{
		Vec2 edge = colliderVerticesA[i + 1] - colliderVerticesA[i];
		axes.push_back(Vec2(-1 * edge.y, edge.x));
	}
	Vec2 lastEdge = colliderVerticesA[0] - colliderVerticesA.back();
	axes.push_back(Vec2(-1 * lastEdge.y, lastEdge.x));

	for (size_t i = 0; i < colliderVerticesB.size() - 1; i++)
	{
		Vec2 edge = colliderVerticesB[i + 1] - colliderVerticesB[i];
		axes.push_back(Vec2(-1 * edge.y, edge.x));
	}
	lastEdge = colliderVerticesB[0] - colliderVerticesB.back();
	axes.push_back(Vec2(-1 * lastEdge.y, lastEdge.x));

	for (auto axis : axes)
	{
		float amin = INFINITE, amax = -1 * INFINITE;
		for (size_t i = 0; i < colliderVerticesA.size(); i++)
		{
			Vec2 p = colliderVerticesA[i];
			float dot = axis.x * p.x + axis.y * p.y;
			if (dot > amax)
			{
				amax = dot;
			}
			if (dot < amin)
			{
				amin = dot;
			}
		}
		float bmin = INFINITE, bmax = -1 * INFINITE;
		for (size_t i = 0; i < colliderVerticesB.size(); i++)
		{
			Vec2 p = colliderVerticesB[i];
			float dot = axis.x * p.x + axis.y * p.y;
			if (dot > bmax)
			{
				bmax = dot;
			}
			if (dot < bmin)
			{
				bmin = dot;
			}
		}

		if ((amin <= bmax && amin >= bmin) || (bmin <= amax && bmin >= amin))
		{
			continue;
		}
		else
		{
			//std::cout << "no collision\n";
			return false;
		}
	}


	//std::cout << "yes!! collision\n";
	return true;
}
