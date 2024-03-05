#include "graham_scan.h"

float ccw(const Vec2& p1, const Vec2& p2, const Vec2& p3)
{
	return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}

std::vector<Vec2> grahamScan(std::vector<Vec2>& points)
{
	// find lowest y-cordinate and left-most point
	Vec2 p0 = points[0];
	for (auto p : points)
	{
		if (p.y < p0.y || (p.y == p0.y && p.x < p0.x))
		{
			p0 = p;
		}
	}

	// sort by polar angle with P0, if several points have the same angle then only keep the farthest
	std::sort(points.begin(), points.end(), [&](const Vec2& p1, const Vec2& p2)
		{
			float angle1 = p0.polar_angle(p1);
			float angle2 = p0.polar_angle(p2);
			if (angle1 != angle2)
			{
				return angle1 < angle2;
			}
			else
			{
				return p0.squaredDist(p1) < p0.squaredDist(p2);
			}
		});
	std::vector<Vec2> polarySortedPoints;
	polarySortedPoints.push_back(points[0]);
	for (size_t i = 1; i < points.size(); ++i)
	{
		if (p0.polar_angle(points[i]) != p0.polar_angle(polarySortedPoints.back()))
		{
			polarySortedPoints.push_back(points[i]);
		}
	}
	polarySortedPoints.insert(polarySortedPoints.begin(), p0);

	// compute convex hull
	std::vector<Vec2> stack;
	for (auto p : points)
	{
		// pop the last point from stack if we turn clockwise to reach this point
		while (stack.size() > 1 && ccw(stack[stack.size() - 2], stack.back(), p) <= 0)
		{
			stack.pop_back();
		}
		stack.push_back(p);
	}

	return stack;
}
