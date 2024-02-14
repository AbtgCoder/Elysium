#pragma once

#include <SFML/Graphics.hpp>

class Vec2
{
public:
	float x = 0;
	float y = 0;

	Vec2();
	Vec2(float xin, float yin);
	Vec2(sf::Vector2f v);
	Vec2(sf::Vector2i v);

	
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


	Vec2 normalize();
	float length();

	float dist(const Vec2& v) const;
};
