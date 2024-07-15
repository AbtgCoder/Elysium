#pragma once

#define MaxPolygonVertices	20

#include "PhysicsShape.h"

class PhysicsPolygonShape : public PhysicsShape
{
public:
	PhysicsPolygonShape();

	void Set(const std::vector<Vec2>& points);

	// build vertices to represent AABB
	void SetAsBox(float hx, float hy);
	void SetAsBox(float hx, float hy, const Vec2& center, float angle);

	void ComputeMass(PhysicsMassData* massData, float density) const override;

public:
	Vec2 m_centroid;
	Vec2 m_vertices[MaxPolygonVertices];
	Vec2 m_normals[MaxPolygonVertices];
	uint32_t m_count;
};

inline PhysicsPolygonShape::PhysicsPolygonShape()
{
	m_type = e_polygon;
	/// radius = polygonRadius: The radius of the polygon/edge shape skin. This should not be modified. Making
	/// this smaller means polygons will have an insufficient buffer for continuous collision.
	/// Making it larger may create artifacts for vertex collision.
	m_count = 0;
	m_centroid = { 0.0f, 0.0f };
}


