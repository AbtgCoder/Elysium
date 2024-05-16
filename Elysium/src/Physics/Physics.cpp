#include "Physics.h"
#include <cmath>

#define INFINITE            0xFFFFFFFF

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

bool Physics::SAT(Entity a, Entity b)
{
	std::vector<Vec2> colliderVerticesA = a.getComponent<CPolygonCollider>().colliderVertices;
	std::vector<Vec2> colliderVerticesB = b.getComponent<CPolygonCollider>().colliderVertices;

	if ((colliderVerticesA.size() == 0 || colliderVerticesB.size() == 0))
	{
		return false;
	}
	if (colliderVerticesA[0] == colliderVerticesB[0])
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


	std::cout << "yes!! collision\n";
	return true;
}
