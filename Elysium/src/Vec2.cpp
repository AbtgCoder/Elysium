#include "Vec2.h"
#include <math.h>

Vec2::Vec2()
{
}

Vec2::Vec2(float xin, float yin)
	: x(xin)
	, y(yin)
{
}

Vec2::Vec2(sf::Vector2f v)
	: x(v.x), y(v.y)
{
}

Vec2::Vec2(sf::Vector2i v)
	: x(v.x), y(v.y)
{
}


Vec2 Vec2::operator+(const Vec2& v) const
{
	return Vec2(x+v.x,y+v.y);
}


Vec2 Vec2::operator*(const float val) const
{
	return Vec2(x*val, y*val);
}

float Vec2::dist(const Vec2& v) const
{	
	float dx = v.x - x;
	float dy = v.y - y;
	return ((dx * dx) + (dy * dy));
	//	return sqrtf((dx*dx)+(dy*dy));
//	return (v - *this).length();
}

void Vec2::operator+=(const Vec2& v)
{
	x += v.x;
	y += v.y;
}

void Vec2::operator-=(const Vec2& v)
{
	x -= v.x;
	y -= v.y;
}

void Vec2::operator*=(const float val)
{
	x *= val;
	y *= val;
}

void Vec2::operator/=(const float val)
{
	x /= val;
	y /= val;
}

Vec2 Vec2::normalize() 
{
	float len = (*this).length();
	return Vec2(x / len, y / len);
}

float Vec2::length()
{
	return sqrtf((x*x)+(y*y));
}

Vec2 Vec2::operator-(const Vec2& v) const
{
	return Vec2(x-v.x,y-v.y);
}

Vec2 Vec2::operator/(const float val) const
{
	return Vec2(x/val,y/val);
}

bool Vec2::operator==(const Vec2& v) const
{
	return (x == v.x) && (y == v.y);
}

bool Vec2::operator!=(const Vec2& v) const
{
	return (x != v.x) || (y != v.y);
}

std::ostream& operator<<(std::ostream& out, const Vec2& v)
{
	out << "(" << v.x << ", " << v.y << ")";
	return out;
}
