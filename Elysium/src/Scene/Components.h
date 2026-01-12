#pragma once


#include "Asset/Asset.h"
#include "Math/Vec2.h"
#include "core/UUID.h"

#include "Scene/SceneCamera.h"

#include "Animation/AnimationController.h"

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


class CId
{
public:
	Elysium::UUID id;
	CId() = default;
	CId(Elysium::UUID uuid)
		: id(uuid) {}
	CId(const CId&) = default;
};

class CTag
{
public:
	std::string tag;
	CTag() {}
	CTag(const std::string& t)
		: tag(t) {}
	CTag(const CTag& other)
		: tag(other.tag) {}
};

class CTransform
{
public:
	glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
	glm::vec3 Scale = {1.0, 1.0, 1.0f};

	glm::vec3 GlobalTranslation = {0.0f, 0.0f, 0.0f};
	glm::vec3 GlobalRotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 GlobalScale = { 1.0f, 1.0f, 1.0f };

	CTransform() {}
	CTransform(const glm::vec3& p)
		: Translation(p) {}
	CTransform(const glm::vec3& p, const glm::vec3& sc, const glm::vec3& a)
		: Translation(p), Scale(sc), Rotation(a) {}
	CTransform(const CTransform& other) = default;

	glm::mat4 GetTransform() const
	{
		glm::mat4 rotation = glm::toMat4(glm::quat(GlobalRotation));

		glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0f), GlobalTranslation)
			* rotation
			* glm::scale(glm::mat4(1.0f), GlobalScale);

		return transformMatrix;
	}
};

class CParent
{
public:
	bool HasParent = false;
	Elysium::UUID ParentID;
	std::vector<Elysium::UUID> Children;

	CParent() = default;
	CParent(const CParent& other)
		: HasParent(other.HasParent), ParentID(other.ParentID), Children(other.Children) {}

	bool RemoveChild(Elysium::UUID childId)
	{
		for (int i = 0; i < Children.size(); i++)
		{
			if (Children[i] == childId)
			{
				Children.erase(Children.begin() + i);
				return true;
			}
		}
		return false;
	}
};

class CCamera
{
public:
	SceneCamera Camera;
	bool primary = true;

	glm::vec4 backgroundColor{ 1.0f, 1.0f, 1.0f, 1.0f };

	CCamera() = default;
	CCamera(const CCamera&) = default;
};

class CAnimator
{
public:
	AnimationController Controller;

	CAnimator() = default;
	CAnimator(const AnimationController& controller)
		: Controller(controller) {}
};

class CScript
{
public:
	std::string ClassName; // name of the script class

	CScript() = default;
	CScript(const CScript&) = default;
};

// forward declaration
class ScriptableEntity;
class CNativeScriptComponent
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

class CRectangle
{
public:
	Vec2 size = { 1.0f, 1.0f};
	//sf::Color color = { 255, 255, 255, 255 };

	glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	CRectangle() {}
	CRectangle(float s)
		: size(s, s) {}
	CRectangle(const Vec2& s)
		: size(s) {}
	CRectangle(const CRectangle& other) = default;

};

class CCircle
{
public:
	float radius = 1.0f;
	//sf::Color color = {255, 255, 255, 255};
	glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };


	CCircle() {}
	CCircle(float r)
		: radius(r) {}
	CCircle(const CCircle& other) = default;
};

class CPolygon
{
public:
	int sides = 3; // this must be >= 3
	float size = 1.0f;
	//sf::Color color = { 255, 255, 255, 255 };
	glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

	CPolygon() = default;
	CPolygon(const CPolygon&) = default;
};

class CLifespan
{
public:
	int lifespan = 0;
	int frameCreated = 0;
	CLifespan() {}
	CLifespan(int duration, int frame)
		: lifespan(duration), frameCreated(frame) {}
	CLifespan(const CLifespan& other)
		: lifespan(other.lifespan), frameCreated(other.frameCreated) {}
};

class CInput
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
	CInput(const CInput& other)
		: up(other.up)
		, left(other.left)
		, right(other.right)
		, down(other.down)
		, shoot(other.shoot)
		, canShoot(other.canShoot)
		, canJump(other.canJump) {}
};

class CBoundingBox
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
	CBoundingBox(const CBoundingBox& other)
		: size(other.size), halfSize(other.halfSize), offset(other.offset) {}
};

class CRigidBody
{
public:
	enum class BodyType { Static = 0, Dynamic, Kinematic};
	BodyType Type = BodyType::Static;

	// storage for runtime
	void* runtimeBody = nullptr;

	CRigidBody() = default;
	CRigidBody(const CRigidBody&) = default;
};

class CCircleCollider
{
public:
	float radius = 0.0f;
	CCircleCollider() {}
	CCircleCollider(float r)
		: radius(r) {}
	CCircleCollider(const CCircleCollider& other)
		: radius(other.radius) {}
};

class CPolygonCollider
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
	CPolygonCollider(const CPolygonCollider& other)
		: size(other.size), colliderVertices(other.colliderVertices), offset(other.offset) {}
};

class CPhysicsMaterial
{
public:
	float mass = 1.0f; // in Kgs //TODO: should be > 0 
	float restitutionCoefficient = 0.0f;
	float friction = 0.02f; // btw 0 and 1
	float restitutionThreshold = 0.5f;

	CPhysicsMaterial() {}
	CPhysicsMaterial(float m)
		: mass(m) {}
	CPhysicsMaterial(float m, float e)
		: mass(m), restitutionCoefficient(e) {}
	CPhysicsMaterial(const CPhysicsMaterial& other) = default;
};

class CJoint
{
public:
	Elysium::UUID entity1Id; // TODO: only works if this is a valid entityID)
	Elysium::UUID entity2Id; // TODO: only works if this is a valid entityID)
	Vec2 anchorPos; // NOTE: this is relative to entity1 pos

	float softness = 0.0f;
	float bias = 0.2f;

	// storage for runtime
	void* runtimeJoint = nullptr;

	bool dirty = true;

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

class CSpriteRenderer
{
public:
	AssetHandle texture = 0;
	int layer = 0;
	CSpriteRenderer() = default;
	CSpriteRenderer(const CSpriteRenderer& other) = default;
};


class CState
{
public:
	std::string state = "standing";
	CState() {}
	CState(const std::string& s)
		: state(s) {}
	CState(const CState& other)
		: state(other.state) {}
};

class CScore
{
public:
	int score = 0;
	CScore() {}
	CScore(int s)
		: score(s) {}
	CScore(const CScore& other)
		: score(other.score) {}
};

