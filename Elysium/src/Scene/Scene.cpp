#include "core/Logger.h"

#include "Scene.h"

#include "Asset/AssetManager.h"
#include "Renderer/Texture.h"
#include "Asset/TextureImporter.h"

#include "Renderer/Renderer2D.h"

#include "Scripting/ScriptEngine.h"

#include "Scripts/RotateEntity.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <windows.h>

// Pixels per meter. Box2D uses metric units, so we need to define a conversion
#define PPM 30.0F
// SFML uses degrees for angles while Box2D uses radians
#define DEG_PER_RAD 57.2957795F


namespace Elysium
{
	SceneCollisionListener::SceneCollisionListener(const std::shared_ptr<Scene>& scene)
		: m_Scene(scene)
	{
	}

	void SceneCollisionListener::OnCollisionBegin(const CollisionEvent& event)
	{
		ScriptEngine::OnCollisionEnter(event);
	}

	void SceneCollisionListener::OnCollisionStay(const CollisionEvent& event)
	{
		ScriptEngine::OnCollisionStay(event);
	}

	void SceneCollisionListener::OnCollisionEnd(const CollisionEvent& event)
	{
		ScriptEngine::OnCollisionExit(event);
	}
}


Scene::Scene()
	: Scene("Untitled")
{
}

Scene::Scene(const std::string& name)
	: m_Name(name)
{

}

Scene::~Scene()
{
	//TODO: delete m_PhysicsWorld;
}

template <typename Component>
static void CopyComponentIfExists(Entity sourceEntity, Entity targetEntity)
{
	if (sourceEntity.hasComponent<Component>())
		targetEntity.addComponent<Component>(sourceEntity.getComponent<Component>());
}

template <typename... Components>
static void CopyComponentsIfExists(Entity sourceEntity, Entity targetEntity)
{
	(CopyComponentIfExists<Components>(sourceEntity, targetEntity), ...);
}

std::shared_ptr<Scene> Scene::Copy(std::shared_ptr<Scene> other)
{
	std::shared_ptr<Scene> scene = std::make_shared<Scene>(other->m_Name);

	scene->m_gravity = other->m_gravity;
	scene->m_externalForce = other->m_externalForce;
	scene->m_drawPhysicsColliders = other->m_drawPhysicsColliders;
	scene->m_velocityIterations = other->m_velocityIterations;
	scene->m_positionIterations = other->m_positionIterations;
	scene->m_KDTreeBroadPhaseCollision = other->m_KDTreeBroadPhaseCollision;

	// create entities in new Scene
	for (auto e : other->m_entityManager.GetEntities())
	{
		auto runtimeEntity = scene->AddEntityWithUUID(e.getComponent<CId>().id, e.getComponent<CTag>().tag);
		runtimeEntity.addComponent<CTransform>(e.getComponent<CTransform>());
		runtimeEntity.addComponent<CParent>(e.getComponent<CParent>());

		CopyComponentsIfExists<
			CSpriteRenderer,
			CCircle,
			CRectangle,
			CPolygon,
			CCircleCollider,
			CBoundingBox,
			CPolygonCollider,
			CRigidBody,
			CPhysicsMaterial,
			CJoint,
			CCamera,
			CNativeScriptComponent,
			CAnimator,
			CScript
		>(e, runtimeEntity);

	}

	scene->m_entityManager.update();

	return scene;
}

Entity Scene::AddEntity(const std::string& name)
{
	return AddEntityWithUUID(Elysium::UUID(), name);
}

Entity Scene::AddEntityWithUUID(Elysium::UUID uuid, const std::string& name)
{
	Entity e = m_entityManager.addEntity();
	e.addComponent<CId>(uuid);
	e.addComponent<CTransform>();
	e.addComponent<CParent>();
	auto& tag = e.addComponent<CTag>();
	tag.tag = name.empty() ? "Entity" : name;
	m_entityManager.update();

	m_EntityMap[uuid] = e; // Store the entity in the map with its UUID

	return e;
}

Entity Scene::AddEntityWithSprite(Vec2 pos, AssetHandle textureHandle)
{
	// asset, asset type as texture
	Entity entity = AddEntity("Tile");
	entity.getComponent<CTransform>().Translation = glm::vec3(pos.x, pos.y, 0.0f);
	entity.addComponent<CSpriteRenderer>();
	entity.getComponent<CSpriteRenderer>().texture = textureHandle;
	return entity;
}

Entity Scene::DuplicateEntity(Entity e, std::optional<Elysium::UUID> newParentID)
{
	// Create a duplicate of the current entity
	auto duplicateEntity = AddEntity(e.getComponent<CTag>().tag);
	duplicateEntity.addComponent<CTransform>(e.getComponent<CTransform>());

	// Handle Parent Component
	auto& originalParentComponent = e.getComponent<CParent>();
	auto& duplicateParentComponent = duplicateEntity.addComponent<CParent>();

	if (newParentID.has_value()) {
		// Assign new parent if specified
		duplicateParentComponent.HasParent = true;
		duplicateParentComponent.ParentID = newParentID.value();

		// Add duplicate entity to the new parent's Children list
		auto newParent = GetEntityByUUID(newParentID.value());
		newParent.getComponent<CParent>().Children.push_back(duplicateEntity.getComponent<CId>().id);
	}
	else if (originalParentComponent.HasParent) {
		// Keep the same parent if no new parent is specified
		duplicateParentComponent.HasParent = true;
		duplicateParentComponent.ParentID = originalParentComponent.ParentID;

		// Add duplicate entity to the original parent's Children list
		auto originalParent = GetEntityByUUID(originalParentComponent.ParentID);
		originalParent.getComponent<CParent>().Children.push_back(duplicateEntity.getComponent<CId>().id);
	}
	else {
		// If the original entity has no parent, the duplicate is a root entity
		duplicateParentComponent.HasParent = false;
	}
	CopyComponentsIfExists<
		CSpriteRenderer,
		CCircle,
		CRectangle,
		CPolygon,
		CCircleCollider,
		CBoundingBox,
		CPolygonCollider,
		CRigidBody,
		CPhysicsMaterial,
		CJoint,
		CCamera,
		CNativeScriptComponent,
		CAnimator,
		CScript
	>(e, duplicateEntity);

	// Handle recursive duplication of children
	for (auto childId : originalParentComponent.Children) {
		Entity originalChildEntity = GetEntityByUUID(childId);

		// Duplicate the child entity and assign it to this duplicate entity
		DuplicateEntity(originalChildEntity, duplicateEntity.getComponent<CId>().id);
	}

	m_entityManager.update();
	return duplicateEntity;
}

Entity Scene::GetEntityByUUID(Elysium::UUID id)
{
	//TODO: assert valid id probably
	//for (auto e : m_entityManager.GetEntities())
	//{
	//	if (e.getComponent<CId>().id == id)
	//	{
	//		return e;
	//	}
	//}

	if (m_EntityMap.find(id) != m_EntityMap.end())
	{
		return m_EntityMap.at(id);
	}
	else
	{
		//Logger::Log("Entity with UUID not found: " + id.ToString(), "Scene", LOG_TYPE::WARNING);
		return Entity(); // Return an invalid entity if not found
	}

}

Entity Scene::GetEntityByEntityID(size_t id)
{
	//TODO: assert valid id probably
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.id() == id)
		{
			return e;
		}
	}
}

Entity Scene::FindEntityByName(const std::string& name)
{
	for (auto entity : m_entityManager.GetEntities())
	{
		if (entity.hasComponent<CTag>() && entity.getComponent<CTag>().tag == name)
		{
			return entity;
		}
	}
	return Entity();
}

void Scene::DestroyEntity(Entity entity)
{
	auto& parentComponent = entity.getComponent<CParent>();
	std::vector<Elysium::UUID> copyChildren = parentComponent.Children;

	if (parentComponent.HasParent)
	{
		// remove self from parents children list..
		auto& parent = GetEntityByUUID(parentComponent.ParentID).getComponent<CParent>();
		parent.RemoveChild(entity.getComponent<CId>().id);
	}

	for (auto& cId : copyChildren)
	{
		DestroyEntity(GetEntityByUUID(cId));
	}

	m_EntityMap.erase(entity.GetUUID()); // Remove the entity from the map using its UUID

	entity.destroy();
}

Camera Scene::GetPrimaryCamera()
{
	// Rendering
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CCamera>())
		{
			auto& camera = e.getComponent<CCamera>();
			if (camera.primary)
			{
				return camera.Camera;
			}
		}
	}
	Logger::Log("there are is no primary camera in scene!", "editor", LOG_TYPE::WARNING);
}

glm::mat4 Scene::GetPrimaryCameraViewMatrix()
{
	// Rendering
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CCamera>())
		{
			auto& camera = e.getComponent<CCamera>();
			if (camera.primary)
			{
				return  glm::inverse(e.getComponent<CTransform>().GetTransform());
			}
		}
	}
	Logger::Log("there are is no primary camera in scene!", "editor", LOG_TYPE::WARNING);
	//TODO: return zero matrix ig idk...
}

bool Scene::IsEntityUUIDValid(Elysium::UUID uuid)
{
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.getComponent<CId>().id == uuid)
		{
			return true;
		}
	}*/
	return m_EntityMap.find(uuid) != m_EntityMap.end();
}

std::vector<Entity>& Scene::GetAllPhysicsEntities()
{
	std::vector<Entity> physicsEntities;
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CBoundingBox>())
		{
			physicsEntities.push_back(e);
		}
	}
	return physicsEntities;
}

ScriptableEntity* TryLoadScript()
{
	std::string userScriptPath = "D:/Game Development/Game_Engine_Programming/Elysium/Sandbox Project/bin/Release-windows-x86_64/Sandbox/Sandbox.dll";
	HMODULE library = LoadLibraryA(userScriptPath.c_str());
	if (!library)
	{
		Logger::Log("couldnt load game scripts!", "Scene", LOG_TYPE::CRITICAL);
		return nullptr;
	}
	using ScriptCreateFunc = ScriptableEntity* (*)();
	std::string funcName = "CreateScript";
	auto createFunc = (ScriptCreateFunc)GetProcAddress(library, funcName.c_str());
	if (!createFunc)
	{
		Logger::Log("failed to find CreateScript function in the game scripts..", "Scene", LOG_TYPE::CRITICAL);
		return nullptr;
	}
	return createFunc();
}

void Scene::CreatePhysicsBody(Entity e)
{
	if (!e.hasComponent<CRigidBody>())
	{
		return;
	}

	auto& rb2d = e.getComponent<CRigidBody>();
	if (rb2d.runtimeBody)
		return; // already created


	auto& transform = e.getComponent<CTransform>();

	PhysicsBody* body = new PhysicsBody();
	body->m_position = { transform.Translation.x, transform.Translation.y };
	body->m_rotation = transform.Rotation.z;
	body->m_type = rb2d.Type == CRigidBody::BodyType::Static ? PhysicsBodyType::staticBody : PhysicsBodyType::dynamicBody;
	body->m_UserData = e.GetUUID(); //TODO: idk if we should use UUID or entity_id for this...

	// Shape
	if (e.hasComponent<CBoundingBox>())
	{
		auto& bb2d = e.getComponent<CBoundingBox>();
		PhysicsPolygonShape* boxShape = new PhysicsPolygonShape();
		boxShape->SetAsBox(bb2d.halfSize.x * transform.Scale.x, bb2d.halfSize.y * transform.Scale.y, bb2d.offset, 0.0f);
		body->m_shape = boxShape;
	}
	else if (e.hasComponent<CPolygonCollider>())
	{
		auto& pc2d = e.getComponent<CPolygonCollider>();
		std::vector<Vec2> vertices = pc2d.colliderVertices;
		std::vector<Vec2> points;
		for (size_t i = 0; i < vertices.size(); i++)
		{
			Vec2 point = vertices[i];
			points.push_back({ point.x, point.y });
		}
		PhysicsPolygonShape* polyShape = new PhysicsPolygonShape();
		polyShape->Set(points);
		body->m_shape = polyShape;
	}
	else if (e.hasComponent<CCircleCollider>())
	{
		PhysicsCircleShape* circleShape = new PhysicsCircleShape();
		circleShape->m_p.Set(0.0f, 0.0f); // TODO: should be offset
		circleShape->m_radius = e.getComponent<CCircleCollider>().radius * e.getComponent<CTransform>().Scale.x;
		body->m_shape = circleShape;
	}
	else
	{
		Logger::Log("Rigidbody added without collider", "Physics", LOG_TYPE::WARNING);
		delete body;
		return;
	}

	// Physics Material
	if (e.hasComponent<CPhysicsMaterial>())
	{
		auto& pm = e.getComponent<CPhysicsMaterial>();
		body->m_friction = pm.friction;
		body->m_restitution = pm.restitutionCoefficient;
	}
	
	body->ResetMassData(5.7f);

	rb2d.runtimeBody = body;
	m_PhysicsWorld->AddBody(body);

}

void Scene::DestroyPhysicsBody(Entity e)
{
	if (!e.hasComponent<CRigidBody>())
		return;

	auto& rb = e.getComponent<CRigidBody>();

	if (!rb.runtimeBody)
		return;

	m_PhysicsWorld->DestroyBody((PhysicsBody*)rb.runtimeBody);
	rb.runtimeBody = nullptr;
}

void Scene::CreatePhysicsHingeJoint(Entity e)
{
	if (!e.hasComponent<CJoint>())
		return;

	auto& jointComp = e.getComponent<CJoint>();
	if (jointComp.runtimeJoint)
		return;

	if (!IsEntityUUIDValid(jointComp.entity2Id))
		return;

	Entity e2 = GetEntityByUUID(jointComp.entity2Id);

	if (!(e.hasComponent<CRigidBody>() && e2.hasComponent<CRigidBody>()))
		return;

	PhysicsBody* body1 = (PhysicsBody*)e.getComponent<CRigidBody>().runtimeBody;
	PhysicsBody* body2 = (PhysicsBody*)e2.getComponent<CRigidBody>().runtimeBody;

	if (!body1 || !body2)
		return;

	if (body1->m_type == PhysicsBodyType::staticBody && body2->m_type == PhysicsBodyType::staticBody)
		return;

	Vec2 anchorWorldPos = jointComp.anchorPos + Vec2(e.getComponent<CTransform>().Translation.x, e.getComponent<CTransform>().Translation.y);

	PhysicsHingeJoint* joint = new PhysicsHingeJoint();
	joint->Set(body1, body2, anchorWorldPos);

	joint->m_softness = jointComp.softness;
	joint->m_biasFactor = jointComp.bias;

	jointComp.runtimeJoint = joint;
	m_PhysicsWorld->AddJoint(joint);

	jointComp.dirty = false;

}

void Scene::DestroyPhysicsHingeJoint(Entity e)
{
	if (!e.hasComponent<CJoint>())
		return;

	auto& jointComp = e.getComponent<CJoint>();
	if (!jointComp.runtimeJoint)
		return;

	m_PhysicsWorld->DestroyJoint(static_cast<PhysicsHingeJoint*>(jointComp.runtimeJoint));

	jointComp.runtimeJoint = nullptr;
}

void Scene::OnRuntimeStart()
{
	//Logger::Log("Starting Runtime");

	m_IsRunning = true;

	UpdateTransforms();

	// Physics
	{
		m_PhysicsWorld = new PhysicsWorld({ 0.0f, -9.8f }, 10);

		// set contact listener, create it here too ig...
		//m_PhysicsWorld->SetContactListener((ContactListener*)(new Elysium::SceneContactListener(this)));
		m_PhysicsWorld->SetContactListener(static_cast<CollisionListener*>(new Elysium::SceneCollisionListener(shared_from_this())));

		for (auto e : m_entityManager.GetEntities())
		{
			CreatePhysicsBody(e);
		}

		for (auto e : m_entityManager.GetEntities())
		{
			CreatePhysicsHingeJoint(e);
		}
	}
	

	// Scripting (doing this after physics , as if someone adds an entity with a rigidbody in OnCreate then that needs scene_>physicsWorld)
	{
		ScriptEngine::OnRuntimeStart(this);

		// instantiate scripts for entities that have them
		for (auto e : m_entityManager.GetEntities())
		{
			if (e.hasComponent<CScript>())
			{
				ScriptEngine::OnCreateEntity(e);
			}
		}
	}

	for (auto joint : m_PhysicsWorld->m_joints)
	{
		auto e1 = GetEntityByUUID(joint->m_body1->m_UserData);
		auto e2 = GetEntityByUUID(joint->m_body2->m_UserData);

		Logger::Log("joint for: " + e1.getComponent<CTag>().tag + " and " + e2.getComponent<CTag>().tag, "scene");

	}

}



void Scene::OnRuntimeStop()
{
	m_IsRunning = false;

	ScriptEngine::OnRuntimeStop();

	// Destroy script
	//for (auto e : m_entityManager.GetEntities())
	//{
	//	if (e.hasComponent<CNativeScriptComponent>())
	//	{
	//		auto& nsc = e.getComponent<CNativeScriptComponent>();
	//		nsc.instance->OnDestroy();
	//		delete nsc.instance;
	//		nsc.instance = nullptr;
	//		//nsc.DestroyScript(&nsc);
	//	}
	//}

	// Physics world deletion
	delete m_PhysicsWorld;
}


void Scene::UpdateTransforms()
{
	// calculate all global transforms..
	for (auto e : m_entityManager.GetEntities())
	{
		auto& transform = e.getComponent<CTransform>();
		if (!e.getComponent<CParent>().HasParent)
		{
			transform.GlobalTranslation = transform.Translation;
			transform.GlobalRotation = transform.Rotation;
			transform.GlobalScale = transform.Scale;
			continue;
		}

		glm::vec3 globalPosition = transform.Translation;
		glm::vec3 globalOrientation = transform.Rotation;
		glm::vec3 globalScale = transform.Scale;

		auto parentComponent = e.getComponent<CParent>();
		while (parentComponent.HasParent)
		{
			auto& parentTransform = GetEntityByUUID(parentComponent.ParentID).getComponent<CTransform>();
			globalPosition += parentTransform.Translation;
			globalOrientation += parentTransform.Rotation;
			globalScale.x *= parentTransform.Scale.x;
			globalScale.y *= parentTransform.Scale.y;
			parentComponent = GetEntityByUUID(parentComponent.ParentID).getComponent<CParent>();
		}
		transform.GlobalTranslation = globalPosition;
		transform.GlobalRotation = globalOrientation;
		transform.GlobalScale = globalScale;
	}
}

void Scene::OnUpdateRuntime(float dt)
{
	if (!m_IsPaused || m_StepFrames-- > 0)
	{
		m_contactPoints.clear();

		// Update Scripts
		{
			for (auto e : m_entityManager.GetEntities())
			{
				if (e.hasComponent<CScript>())
				{
					ScriptEngine::OnUpdateEntity(e, dt);
				}
			}

			/*for (auto e : m_entityManager.GetEntities())
			{
				if (e.hasComponent<CNativeScriptComponent>())
				{
					auto& nsc = e.getComponent<CNativeScriptComponent>();
					if (nsc.instance)
						nsc.instance->OnUpdate(dt);
				}
			}*/
		}

		// Physics
		{
			// update joints
			for (auto e : m_entityManager.GetEntities())
			{
				if (e.hasComponent<CJoint>() && e.getComponent<CJoint>().dirty)
					CreatePhysicsHingeJoint(e);
			}

			m_PhysicsWorld->Update(dt);

			//m_PhysicsWorld->Step(dt);

			// Debug: Display contact points
			std::map<ArbiterKey, Arbiter>::const_iterator iter;
			for (iter = m_PhysicsWorld->m_arbiters.begin(); iter != m_PhysicsWorld->m_arbiters.end(); ++iter)
			{
				const Arbiter& arbiter = iter->second;
				for (int i = 0; i < arbiter.m_numContacts; ++i)
				{
					Vec2 p = arbiter.m_contacts[i].m_position;
					m_contactPoints.push_back(p);
				}
			}

			for (auto e : m_entityManager.GetEntities())
			{
				if (e.hasComponent<CRigidBody>())
				{
					auto& rb2d = e.getComponent<CRigidBody>();
					if (rb2d.runtimeBody)
					{
						//TODO: fix this for when entity e has a parent....
						PhysicsBody* body = (PhysicsBody*)rb2d.runtimeBody;
						auto& transform = e.getComponent<CTransform>();
						transform.Translation.x = body->m_position.x;
						transform.Translation.y = body->m_position.y;
						transform.Rotation.z = body->m_rotation;
					}
				}
			}

		}
		
		// Animation
		for (auto e : m_entityManager.GetEntities())
		{
			if (e.hasComponent<CAnimator>())
			{
				auto& animController = e.getComponent<CAnimator>().Controller;
				animController.Update(dt);
			}
		}
	}

	// recompute every entity's global transform
	UpdateTransforms();


	// Rendering
	Camera* mainCamera = nullptr;
	CTransform cameraTransform;
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CCamera>())
		{
			auto& camera = e.getComponent<CCamera>();
			if (camera.primary)
			{
				mainCamera = &camera.Camera;
				cameraTransform = e.getComponent<CTransform>();
				break;
			}
		}
	}

	if (mainCamera)
	{

		Renderer2D::BeginScene(*mainCamera, cameraTransform.GetTransform());

		for (auto entity : m_entityManager.GetEntities())
		{
			auto transform = entity.getComponent<CTransform>();

			if (entity.hasComponent<CRectangle>())
			{
				auto rect = entity.getComponent<CRectangle>();
				glm::mat4 t = entity.getComponent<CTransform>().GetTransform() * glm::scale(glm::mat4(1.0f), { rect.size.x, rect.size.y, 1.0f });

				Renderer2D::DrawQuad(t, rect.color, (int)entity.id());

				//Renderer2D::DrawRotatedQuad({ transform.GlobalTranslation.x, transform.GlobalTranslation.y }, { rect.size.x, rect.size.y }, transform.GlobalRotation.z, rect.color, (int)entity.id());
			}

			if (entity.hasComponent<CCircle>())
			{
				auto circle = entity.getComponent<CCircle>();
				Renderer2D::DrawCircle({ transform.GlobalTranslation.x, transform.GlobalTranslation.y }, circle.radius, circle.color, (int)entity.id());
			}

			if (entity.hasComponent<CPolygon>())
			{
				auto polygon = entity.getComponent<CPolygon>();
				Renderer2D::DrawPolygon(transform.GetTransform(), polygon.size, polygon.sides, polygon.color, (int)entity.id());
			}

			bool renderSpriteFromSpriteRenderer = true;
			if (entity.hasComponent<CAnimator>())
			{
				auto& animController = entity.getComponent<CAnimator>().Controller;

				if (animController.m_States.contains(animController.m_CurrentState))
				{
					const auto& clip = animController.m_States.at(animController.m_CurrentState).Clip;
					if (clip && clip->m_SpriteSheetTexture)
					{
						renderSpriteFromSpriteRenderer = false;

						auto animFrame = animController.GetCurrentFrame();

						// draw quad with texture and frames's uv coordinates
						Renderer2D::DrawQuad(transform.GetTransform(), clip->m_SpriteSheetTexture, glm::vec4(1.0f), animFrame.UVmin, animFrame.UVMax, (int)entity.id());
					}
				}

			}
			if (entity.hasComponent<CSpriteRenderer>() && renderSpriteFromSpriteRenderer)
			{
				auto& src = entity.getComponent<CSpriteRenderer>();

				if (src.texture != 0)
				{
					std::shared_ptr<Texture2D> texture = AssetManager::GetAsset<Texture2D>(src.texture);
					Renderer2D::DrawQuad(transform.GetTransform(), texture, glm::vec4(1.0f),(int)entity.id());
				}
				else
				{
					Renderer2D::DrawQuad(transform.GetTransform(), glm::vec4(1.0f), (int)entity.id());
				}
			}
		}


		// debug draw : physics contact points
		for (auto cp : m_contactPoints)
		{
			//Logger::Log("contact point: (" + std::to_string(cp.x) + ", " + std::to_string(cp.y) + ")", "Scene");
			Renderer2D::DrawCircle({ cp.x, cp.y }, 0.05f, {1.0f, 0.0f, 0.0f, 1.0f}, (int)(cp.x));
		}
		for (auto joint : m_PhysicsWorld->m_joints)
		{
			PhysicsBody* b1 = joint->m_body1;
			PhysicsBody* b2 = joint->m_body2;

			Mat22 r1(b1->m_rotation);
			Mat22 r2(b2->m_rotation);

			Vec2 x1 = b1->m_position;
			Vec2 p1 = x1 + r1 * joint->m_localAnchor1;

			Vec2 x2 = b2->m_position;
			Vec2 p2 = x2 + r2 * joint->m_localAnchor2;

			Renderer2D::DrawLine({x1.x, x1.y}, {p1.x, p1.y}, {0.5f, 0.5f, 0.8f, 1});
			Renderer2D::DrawLine({p1.x, p1.y}, {x2.x, x2.y}, {0.5f, 0.5f, 0.8f, 1});
			Renderer2D::DrawLine({x2.x, x2.y}, {p2.x, p2.y}, {0.5f, 0.5f, 0.8f, 1});
		}

		Renderer2D::EndScene();
	}
	else
	{
		Logger::Log("there are is no primary camera in scene!", "editor", LOG_TYPE::WARNING);
	}

	
}

void Scene::OnUpdateEditor(EditorCamera& camera)
{
	// Update transforms..
	UpdateTransforms();

	//TODO: maybe need some way to play animations in editing mode...

	// Render Scene
	RenderScene(camera);
}

void Scene::OnViewportResize(uint32_t width, uint32_t height)
{
	if (m_ViewportWidth == width && m_ViewportHeight == height)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;

	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CCamera>())
		{
			auto& cameraComponent = e.getComponent<CCamera>();
			cameraComponent.Camera.SetViewportSize(width, height);
		}
	}
}



void Scene::Step(int frames)
{
	m_StepFrames = frames;
}

void Scene::RenderScene(EditorCamera& camera)
{
	Renderer2D::BeginScene(camera);

	for (auto entity : m_entityManager.GetEntities())
	{
		auto transform = entity.getComponent<CTransform>();

		if (entity.hasComponent<CRectangle>())
		{
			auto rect = entity.getComponent<CRectangle>();
			glm::mat4 t = entity.getComponent<CTransform>().GetTransform() * glm::scale(glm::mat4(1.0f), { rect.size.x, rect.size.y, 1.0f });

			Renderer2D::DrawQuad(t, rect.color, (int)entity.id());

			/*auto rect = entity.getComponent<CRectangle>();
			Renderer2D::DrawRotatedQuad({ transform.GlobalTranslation.x, transform.GlobalTranslation.y }, { rect.size.x, rect.size.y }, transform.GlobalRotation.z, rect.color, (int)entity.id());*/
		}

		if (entity.hasComponent<CCircle>())
		{
			auto circle = entity.getComponent<CCircle>();
			Renderer2D::DrawCircle({ transform.GlobalTranslation.x, transform.GlobalTranslation.y }, circle.radius, circle.color, (int)entity.id());
		}

		if (entity.hasComponent<CPolygon>())
		{
			auto polygon = entity.getComponent<CPolygon>();
			Renderer2D::DrawPolygon(transform.GetTransform(), polygon.size, polygon.sides, polygon.color, (int)entity.id());
		}

		if (entity.hasComponent<CSpriteRenderer>())
		{
			auto& src = entity.getComponent<CSpriteRenderer>();

			if (src.texture != 0)
			{
				std::shared_ptr<Texture2D> texture = AssetManager::GetAsset<Texture2D>(src.texture);
				Renderer2D::DrawQuad(transform.GetTransform(), texture, glm::vec4(1.0f), (int)entity.id());
			}
			else
			{
				Renderer2D::DrawQuad(transform.GetTransform(), glm::vec4(1.0f), (int)entity.id());
			}
		}
	}


#if 0
	//TODO: editor setting for draw joints should be enabled to see this ig...
	for (auto joint : m_PhysicsWorld->m_joints)
	{
		PhysicsBody* b1 = joint->m_body1;
		PhysicsBody* b2 = joint->m_body2;

		Mat22 r1(b1->m_rotation);
		Mat22 r2(b2->m_rotation);

		Vec2 x1 = b1->m_position;
		Vec2 p1 = x1 + r1 * joint->m_localAnchor1;

		Vec2 x2 = b2->m_position;
		Vec2 p2 = x2 + r2 * joint->m_localAnchor2;

		Renderer2D::DrawLine({ x1.x, x1.y }, { p1.x, p1.y }, { 1, 0, 0, 1 });
		Renderer2D::DrawLine({ p1.x, p1.y }, { x2.x, x2.y }, { 1, 0, 0, 1 });
		Renderer2D::DrawLine({ x2.x, x2.y }, { p2.x, p2.y }, { 1, 0, 0, 1 });
	}
#endif

	Renderer2D::EndScene();
}
