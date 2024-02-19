#pragma once

#include "Animation.h"
#include "Assets.h"

class Component
{
public:
	bool has = false;
};

class CTransform : public Component
{
public:
	Vec2 pos = { 0.0, 0.0 };
	Vec2 prevPos = { 0.0, 0.0 };
	Vec2 velocity = { 0.0, 0.0 };
	Vec2 scale = {1.0, 1.0};
	float angle=0;

	CTransform() {}
	CTransform(const Vec2& p)
		: pos(p), prevPos(p) {}
	CTransform(const Vec2& p, const Vec2& sc, float a)
		: pos(p), prevPos(p), scale(sc), angle(a) {}
	CTransform(const Vec2& p, const Vec2& sp, const Vec2& sc, float a)
		: pos(p), prevPos(p), velocity(sp), scale(sc), angle(a) {}
	CTransform(CTransform& other)
		: pos(other.pos), prevPos(other.prevPos), velocity(other.velocity), scale(other.scale), angle(other.angle) {}
};

class CLifespan : public Component
{
public:
	int lifespan = 0;
	int frameCreated = 0;
	CLifespan() {}
	CLifespan(int duration, int frame)
		: lifespan(duration), frameCreated(frame) {}
	CLifespan(CLifespan& other)
		: lifespan(other.lifespan), frameCreated(other.frameCreated) {}
};

class CInput : public Component
{
public:
	bool up = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool shoot = false;
	bool canShoot = true;
	bool canJump = true;
	
	CInput() {}
	CInput(CInput& other)
		: up(other.up)
		, left(other.left)
		, right(other.right)
		, down(other.down)
		, shoot(other.shoot)
		, canShoot(other.canShoot)
		, canJump(other.canJump) {}
};

class CBoundingBox : public Component
{
public:
	Vec2 size;
	Vec2 halfSize;
	Vec2 offset;
	CBoundingBox() {}
	CBoundingBox(const Vec2& s)
		: size(s), halfSize(s.x/2,s.y/2) {}
	CBoundingBox(const Vec2& s, const Vec2& o)
		: size(s), halfSize(s.x / 2, s.y / 2), offset(o) {}
	CBoundingBox(CBoundingBox& other)
		: size(other.size), halfSize(other.halfSize), offset(other.offset) {}
};

class CAnimation : public Component
{
public:
	Animation animation;
	size_t animSpeed = 0;
	size_t frameCount = 1;
	bool repeat = false; // repetable animation or not ??
	int layer = 0; // rendering layer
	CAnimation() {}
	CAnimation(const Animation& animation, bool r)
		: animation(animation), repeat(r) {}
	CAnimation(const Animation& animation, bool r, int l)
		: animation(animation), repeat(r), layer(l) {}
	CAnimation(CAnimation& other)
		: animation(other.animation), repeat(other.repeat), layer(other.layer) {}
};

class CGravity : public Component
{
public:
	float gravity = 0;
	CGravity() {}
	CGravity(float g) 
		: gravity(g) {}
	CGravity(CGravity& other)
		: gravity(other.gravity) {}
};

class CState : public Component
{
public:
	std::string state = "standing";
	CState() {}
	CState(const std::string& s)
		: state(s) {}
	CState(CState& other)
		: state(other.state) {}
};

class CScore : public Component
{
public:
	int score = 0;
	CScore() {}
	CScore(int s)
		: score(s) {}
	CScore(CScore& other)
		: score(other.score) {}
};

class CDraggable : public Component
{
public:
	bool dragging = false;
	CDraggable() {}
	CDraggable(bool d)
		: dragging(d) {}
	CDraggable(CDraggable& other)
		: dragging(true) {}
};

#if 0
class CShape
{
public:
	sf::CircleShape circle;

	CShape(float radius, int points, const sf::Color& fill, const sf::Color& outline, float thickness)
		: circle(radius, points)
	{
		circle.setFillColor(fill);
		circle.setOutlineColor(outline);
		circle.setOutlineThickness(thickness);
		circle.setOrigin(radius, radius);
	}
};

class CConvexShape
{
public:
	sf::ConvexShape convex;
	CConvexShape(int points, const sf::Color& fill, const sf::Color& outline, float thickness)
	{
		convex.setPointCount(points);
		convex.setFillColor(fill);
		convex.setOutlineColor(outline);
		convex.setOutlineThickness(thickness);
	}
};


class CCollision
{
public:
	float radius = 0; // Collision Radius ??
	CCollision(float r)
		: radius(r) {}
};




class CLives
{
public:
	int remaining = 0;
	int total = 0;
	CLives(int total)
		: remaining(total), total(total) {}
};


#endif
