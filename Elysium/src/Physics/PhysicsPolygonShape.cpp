#include "PhysicsPolygonShape.h"

void PhysicsPolygonShape::Set(const std::vector<Vec2>& points)
{
	m_count = points.size();

	// asssrt m_count >= 3

	// copy verties
	for (size_t i = 0; i < m_count; i++)
	{
		m_vertices[i] = points[i];
	}


	// compute centroid (are-weighted)
	Vec2 c(0.0f, 0.0f);
	float area = 0.0f;
	for (int i = 0; i < m_count; ++i)
	{
		const Vec2& p0 = m_vertices[i];
		const Vec2& p1 = m_vertices[(i + 1) % m_count];

		float D = Cross(p0, p1);
		float triangleArea = 0.5f * D;

		area += triangleArea;
		c += (p0 + p1) * (triangleArea / 3.0f);
	}

	c = c * (1.0f / area);
	m_centroid = c;

	// shift vertices so centroid is at origin
	for (int i = 0; i < m_count; ++i)
		m_vertices[i] -= m_centroid;

	// compute edge normals(outwards)
	for (int i = 0; i < m_count; i++)
	{
		Vec2 v1 = m_vertices[i];
		Vec2 v2 = m_vertices[(i + 1) % m_count];
		Vec2 edge = v2 - v1;

		// perpendicular CCW edge -> outward normal
		m_normals[i] = Vec2(edge.y, -edge.x).normalize();
	}
}

// Box vertex and edge numbering:
// 
//   v1 ------ v2
//    |        |
//	  |        |   --> x
//    |        |
//   v4 ------ v3
//        

void PhysicsPolygonShape::SetAsBox(float hx, float hy)
{
	m_count = 4;
	m_vertices[0].Set(-hx, -hy);
	m_vertices[1].Set(hx, -hy);
	m_vertices[2].Set(hx, hy);
	m_vertices[3].Set(-hx, hy);
	m_normals[0].Set(0.0f, -1.0f);
	m_normals[1].Set(1.0f, 0.0f);
	m_normals[2].Set(0.0f, 1.0f);
	m_normals[3].Set(-1.0f, 0.0f);
	m_centroid.Set(0.0f, 0.0f);
}

void PhysicsPolygonShape::SetAsBox(float hx, float hy, const Vec2& center, float angle) //NOTE: this angle is local rotation
{
	m_count = 4;
	m_vertices[0].Set(-hx, -hy);
	m_vertices[1].Set(hx, -hy);
	m_vertices[2].Set(hx, hy);
	m_vertices[3].Set(-hx, hy);
	m_normals[0].Set(0.0f, -1.0f);
	m_normals[1].Set(1.0f, 0.0f);
	m_normals[2].Set(0.0f, 1.0f);
	m_normals[3].Set(-1.0f, 0.0f);
	m_centroid = center; // this is offset


	// transform vertices and normals
	float rad = angle * (3.14 / 180.0f);
	float cosA = std::cos(rad);
	float sinA = std::sin(rad);
	for (uint32_t i = 0; i < m_count; ++i)
	{
		m_vertices[i] = { m_centroid.x + (m_vertices[i].x * cosA - m_vertices[i].y * sinA),  m_centroid.y + (m_vertices[i].x * sinA + m_vertices[i].y * cosA) };
		m_normals[i] = { m_normals[i].x * cosA - m_normals[i].y * sinA, m_normals[i].x * sinA + m_normals[i].y * cosA};
	}
}

void PhysicsPolygonShape::ComputeMass(PhysicsMassData* massData, float density) const
{
	//TODO: mass and moment of inertia calculation for polygons

	Vec2 c(0.0f, 0.0f);
	float area = 0.0f;
	float I = 0.0f;

	const float k_inv3 = 1.0f / 3.0f;

	for (int i = 0; i < m_count; ++i)
	{
		const Vec2& p0 = m_vertices[i];
		const Vec2& p1 = m_vertices[(i + 1) % m_count];

		float D = Cross(p0, p1);
		float triangleArea = 0.5f * D;

		area += triangleArea;
		c += (p0 + p1) * (triangleArea * k_inv3);

		float intx2 = p0.x * p0.x + p1.x * p1.x + p0.x * p1.x;
		float inty2 = p0.y * p0.y + p1.y * p1.y + p0.y * p1.y;

		I += (0.25f * k_inv3 * D) * (intx2 + inty2);
	}
	c *= (1.0f / area);

	massData->mass = density * area;
	massData->center = c;
	massData->I = density * I;

	// massData->mass = 50.0f;
	//massData->I = 50.0f;
}
