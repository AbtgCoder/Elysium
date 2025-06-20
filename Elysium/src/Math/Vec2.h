#pragma once

#include <ostream>

#include <vector>

class Vec2
{
public:
	float x = 0;
	float y = 0;

	Vec2() : x(0), y(0) {}
	Vec2(float xin, float yin);
	Vec2(const Vec2& v) = default;

	void Set(float xin, float yin) { x = xin; y = yin; }

	float operator[](size_t i) const;
	
	bool operator == (const Vec2& v) const;
	bool operator != (const Vec2& v) const;

	Vec2 operator + (const Vec2& v) const;
	Vec2 operator * (const float val) const;
	Vec2 operator - (const Vec2& v) const;
	Vec2 operator / (const float val) const;

	void operator += (const Vec2& v);
	void operator -= (const Vec2& v);
	void operator *= (const float val);
	void operator /= (const float val);

	friend std::ostream& operator << (std::ostream& out, const Vec2& v);


	Vec2 perpendicular() const;

	Vec2 normalize();
	float length();

	float dist(const Vec2& v) const;
	float squaredDist(const Vec2& v) const;

	double dot(const Vec2& v) const;
	float cross(const Vec2& v) const;

	Vec2 rotate(float angle) const;

	float polar_angle(const Vec2& v) const;
};

inline float Cross(const Vec2& a, const Vec2& b)
{
	return a.x * b.y - a.y * b.x;
}

inline Vec2 Cross(const Vec2& a, float s)
{
	return Vec2(s * a.y, -s * a.x);
}

inline Vec2 Cross(float s, const Vec2& a)
{
	return Vec2(-s * a.y, s * a.x);
}

