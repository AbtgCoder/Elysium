#pragma once


#include "Asset/Asset.h"
#include "Math/Vec2.h"
#include "core/UUID.h"

class Component
{
public:
	bool has = false;
};

class CId : public Component
{
public:
	Elysium::UUID id;
	CId() = default;
	CId(Elysium::UUID uuid)
		: id(uuid) {}
	CId(const CId&) = default;
};

class CTag : public Component
{
public:
	std::string tag;
	CTag() {}
	CTag(const std::string& t)
		: tag(t) {}
	CTag(CTag& other)
		: tag(other.tag) {}
};

class CTransform : public Component
{
public:
	Vec2 pos = { 0.0, 0.0 };
	Vec2 prevPos = { 0.0, 0.0 };
	Vec2 velocity = { 0.0, 0.0 };
	Vec2 scale = {1.0, 1.0};
	float angle = 0.0f;
	float angularVelocity = 0.0f;

	CTransform() {}
	CTransform(const Vec2& p)
		: pos(p), prevPos(p) {}
	CTransform(const Vec2& p, const Vec2& sc, float a)
		: pos(p), prevPos(p), scale(sc), angle(a) {}
	CTransform(const Vec2& p, const Vec2& sp, const Vec2& sc, float a)
		: pos(p), prevPos(p), velocity(sp), scale(sc), angle(a) {}
	CTransform(CTransform& other) = default;
};

// forward declaration
class ScriptableEntity;
class CNativeScriptComponent : public Component
{
public:
	ScriptableEntity* instance = nullptr;

	// function pointers
	ScriptableEntity* (*InstantiateScript)(); // function InstantiateScript , return type -> ScriptableEntity*
	void (*DestroyScript)(CNativeScriptComponent*); // function DestroyScript , return type -> void , parameter: NativeScriptComponent*

	template<typename T>
	void Bind()
	{
		InstantiateScript = []() {return static_cast<ScriptableEntity*>(new T()); };
		DestroyScript = [](CNativeScriptComponent* nsc) { delete nsc->instance; nsc->instance = nullptr; };
	}
};

class CRectangle : public Component
{
public:
	Vec2 size = {50.0, 50.0};
	sf::Color color = { 255, 255, 255, 255 };

	CRectangle() {}
	CRectangle(float s)
		: size(s, s) {}
	CRectangle(const Vec2& s)
		: size(s) {}
	CRectangle(const CRectangle& other) = default;

};

class CCircle : public Component
{
public:
	float radius = 50.0f;
	sf::Color color = {255, 255, 255, 255};
	CCircle() {}
	CCircle(float r)
		: radius(r) {}
	CCircle(CCircle& other) = default;
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
	Vec2 offset = {0.0f, 0.0f};
	CBoundingBox() {}
	CBoundingBox(const Vec2& s)
		: size(s), halfSize(s.x/2,s.y/2) {}
	CBoundingBox(const Vec2& s, const Vec2& o)
		: size(s), halfSize(s.x / 2, s.y / 2), offset(o) {}
	CBoundingBox(CBoundingBox& other)
		: size(other.size), halfSize(other.halfSize), offset(other.offset) {}
};

class CRigidBody : public Component
{
public:
	enum class BodyType { Static = 0, Dynamic, Kinematic};
	BodyType Type = BodyType::Static;

	// storage for runtime
	void* runtimeBody = nullptr;

	CRigidBody() = default;
	CRigidBody(const CRigidBody&) = default;
};

class CCircleCollider : public Component
{
public:
	float radius = 0.0f;
	CCircleCollider() {}
	CCircleCollider(float r)
		: radius(r) {}
	CCircleCollider(CCircleCollider& other)
		: radius(other.radius) {}
};

class CPolygonCollider : public Component
{
public:
	std::vector<Vec2> colliderVertices;
	Vec2 offset = {0.0f, 0.0f};
	Vec2 size;
	CPolygonCollider() {}
	CPolygonCollider(const std::vector<Vec2>& vertices)
		: colliderVertices(vertices) {}
	CPolygonCollider(const Vec2& s, const std::vector<Vec2>& vertices)
		: size(s), colliderVertices(vertices) {}
	CPolygonCollider(CPolygonCollider& other)
		: size(other.size), colliderVertices(other.colliderVertices), offset(other.offset) {}
};

class CPhysicsMaterial : public Component
{
public:
	float mass = 1.0f; // in Kgs //TODO: should be > 0 
	float restitutionCoefficient = 1.0f;
	float friction = 0.02f; // btw 0 and 1

	CPhysicsMaterial() {}
	CPhysicsMaterial(float m)
		: mass(m) {}
	CPhysicsMaterial(float m, float e)
		: mass(m), restitutionCoefficient(e) {}
	CPhysicsMaterial(CPhysicsMaterial& other) = default;
};

class CJoint : public Component
{
public:
	Elysium::UUID entity1Id; // TODO: only works if this is a valid entityID)
	Elysium::UUID entity2Id; // TODO: only works if this is a valid entityID)
	Vec2 anchorPos; // NOTE: this is relative to entity1 pos

	CJoint() = default;
	CJoint(Elysium::UUID id)
		: entity1Id(id) {}
	CJoint(Elysium::UUID id, const Vec2& aPos)
		: entity1Id(id), anchorPos(aPos) {}
	CJoint(Elysium::UUID id1, Elysium::UUID id2)
		: entity1Id(id1), entity2Id(id2) {}
	CJoint(Elysium::UUID id1, Elysium::UUID id2, const Vec2& aPos)
		: entity1Id(id1), entity2Id(id2), anchorPos(aPos) {}
};

class CSpriteRenderer : public Component
{
public:
	AssetHandle texture = 0;
	int layer = 0;
	CSpriteRenderer() = default;
	CSpriteRenderer(const CSpriteRenderer& other) = default;
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

