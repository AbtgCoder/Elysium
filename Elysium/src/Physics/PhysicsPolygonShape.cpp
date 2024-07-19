#include "PhysicsPolygonShape.h"

void PhysicsPolygonShape::Set(const std::vector<Vec2>& points)
{
	for (size_t i = 0; i < points.size(); i++)
	{
		m_vertices[i] = points[i];
	}
	m_count = points.size();
	//TODO: compute and store normals

	m_centroid.Set(0.0f, 0.0f);//TODO: compute centroid

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
	massData->mass = 1.0f;
	massData->I = 1.0f;
}
