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
		auto runtimeEntity = scene->AddEntityWithUUID(e.getComponent<CId>().id, "runtime_" + e.getComponent<CTag>().tag);
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
	auto entity = m_entityManager.addEntity();
	entity.addComponent<CId>(Elysium::UUID());
	entity.addComponent<CParent>();
	entity.addComponent<CTag>("Tile");
	entity.addComponent<CTransform>(glm::vec3(pos.x, pos.y, 0.0f));
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
}

bool Scene::IsEntityUUIDValid(Elysium::UUID uuid)
{
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.getComponent<CId>().id == uuid)
		{
			return true;
		}
	}
	return false;
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

void Scene::OnRuntimeStart()
{
	//Logger::Log("Starting Runtime");

	m_IsRunning = true;

	UpdateTransforms();

	// Scripting
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

	// Instantiate script
	//for (auto e : m_entityManager.GetEntities())
	//{
	//	if (e.hasComponent<CNativeScriptComponent>())
	//	{
	//		auto& nsc = e.getComponent<CNativeScriptComponent>();
	//		//nsc.instance = nsc.InstantiateScript();
	//		nsc.instance = TryLoadScript();
	//		nsc.instance->m_Entity = e;
	//		nsc.instance->m_EntityManager = &m_entityManager;
	//		nsc.instance->OnCreate();
	//	}
	//}

	// Physics world initialization
	m_PhysicsWorld = new PhysicsWorld({0.0f, -9.8f}, 10);
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CRigidBody>())
		{
			auto& transform = e.getComponent<CTransform>();
			auto& rb2d = e.getComponent<CRigidBody>();

			PhysicsBody* body = new PhysicsBody();
			body->m_position = { transform.GlobalTranslation.x, transform.GlobalTranslation.y };
			body->m_rotation = transform.GlobalRotation.z;
		//	body->m_velocity = transform.velocity;
		//	body->m_angularVelocity = transform.angularVelocity;
			body->m_type = rb2d.Type == CRigidBody::BodyType::Static ? PhysicsBodyType::staticBody : PhysicsBodyType::dynamicBody;
			rb2d.runtimeBody = body;

			if (e.hasComponent<CBoundingBox>())
			{
				auto& bb2d = e.getComponent<CBoundingBox>();
				PhysicsPolygonShape* boxShape = new PhysicsPolygonShape();
				boxShape->SetAsBox(bb2d.halfSize.x * transform.GlobalScale.x, bb2d.halfSize.y * transform.GlobalScale.y, bb2d.offset, 0.0f);
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
				circleShape->m_radius = e.getComponent<CCircleCollider>().radius * e.getComponent<CTransform>().GlobalScale.x;
				body->m_shape = circleShape;
			}

			if (e.hasComponent<CPhysicsMaterial>())
			{
				auto& pm = e.getComponent<CPhysicsMaterial>();
				body->m_friction = pm.friction;
				body->m_restitution = pm.restitutionCoefficient;
				body->ResetMassData(5.7f);
			}
			else
			{
				body->ResetMassData(5.7f);
			}

			m_PhysicsWorld->AddBody(body);
		}
		

	}
	
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CJoint>())
		{
			auto& jointComponent = e.getComponent<CJoint>();
			if (!IsEntityUUIDValid(jointComponent.entity2Id))
			{
				continue;
			}
			auto entity2 = GetEntityByUUID(jointComponent.entity2Id);
			if (!(e.hasComponent<CRigidBody>() && entity2.hasComponent<CRigidBody>())) //TODO: should we be doing these checks here ??
			{
				continue;
			}
			PhysicsBody* body1 = (PhysicsBody*)(e.getComponent<CRigidBody>().runtimeBody);
			PhysicsBody* body2 = (PhysicsBody*)(entity2.getComponent<CRigidBody>().runtimeBody);
			if (body1->m_type == PhysicsBodyType::staticBody && body2->m_type == PhysicsBodyType::staticBody)
			{
				continue;
			}
			Vec2 anchorWorldPos = jointComponent.anchorPos + Vec2(e.getComponent<CTransform>().GlobalTranslation.x, e.getComponent<CTransform>().GlobalTranslation.y);
			PhysicsHingeJoint* joint = new PhysicsHingeJoint();
			joint->Set(body1, body2, Vec2(anchorWorldPos.x, anchorWorldPos.y));
			// TODO: add softness & bias to the joint component ??
			joint->m_softness = 0.00098884f;
			joint->m_biasFactor = 0.130132;
			jointComponent.runtimeJoint = joint;
			m_PhysicsWorld->AddJoint(joint);
		}
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
	if (m_bomb)
		m_bomb.destroy();
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

			m_PhysicsWorld->Step(dt);

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
				Renderer2D::DrawRotatedQuad({ transform.GlobalTranslation.x, transform.GlobalTranslation.y }, { rect.size.x, rect.size.y }, transform.GlobalRotation.z, rect.color, (int)entity.id());
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
			Renderer2D::DrawRotatedQuad({ transform.GlobalTranslation.x, transform.GlobalTranslation.y }, { rect.size.x, rect.size.y }, transform.GlobalRotation.z, rect.color, (int)entity.id());
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

	Renderer2D::EndScene();


#if 0
	CCamera* mainCamera = nullptr;
	CTransform cameraTransform;
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CCamera>())
		{
			auto camera = e.getComponent<CCamera>();
			if (camera.primary)
			{
				mainCamera = &camera;
				cameraTransform = e.getComponent<CTransform>();
				break;
			}
		}
	}


	for (auto e : m_entityManager.GetEntities())
		{
			if (m_drawPhysicsColliders)
			{
				if (e.hasComponent<CCircleCollider>())
				{
					m_CircleShape.setPointCount(30);
					m_CircleShape.setRadius(e.getComponent<CCircleCollider>().radius);
					m_CircleShape.setOrigin(e.getComponent<CCircleCollider>().radius, e.getComponent<CCircleCollider>().radius);
					m_CircleShape.setPosition(e.getComponent<CTransform>().GlobalTranslation.x, e.getComponent<CTransform>().GlobalTranslation.y);
					m_CircleShape.setRotation(-1 * e.getComponent<CTransform>().GlobalRotation);
					m_CircleShape.setFillColor(sf::Color::Transparent);
					m_CircleShape.setOutlineColor(e.getComponent<CCircle>().color);
					m_CircleShape.setOutlineThickness(1.0f);
					renderTexture.draw(m_CircleShape);
				}
				else if (e.hasComponent<CBoundingBox>())
				{
					Vec2 rectSize = e.getComponent<CBoundingBox>().size;
					m_PhysicsRect.setSize(sf::Vector2f(rectSize.x, rectSize.y));
					m_PhysicsRect.setOrigin(rectSize.x / 2, rectSize.y / 2);
					m_PhysicsRect.setRotation(-1 * e.getComponent<CTransform>().GlobalRotation);
					m_PhysicsRect.setPosition(e.getComponent<CTransform>().GlobalTranslation.x + e.getComponent<CBoundingBox>().offset.x, e.getComponent<CTransform>().GlobalTranslation.y + e.getComponent<CBoundingBox>().offset.y);
					m_PhysicsRect.setFillColor(sf::Color::Transparent);
					m_PhysicsRect.setOutlineColor(e.getComponent<CRectangle>().color);
					renderTexture.draw(m_PhysicsRect);
				}
				else if (e.hasComponent<CPolygonCollider>())
				{
					std::vector<Vec2> vertices = e.getComponent<CPolygonCollider>().colliderVertices;
					m_PhysicsPoly.setPointCount(vertices.size());
					Vec2 ePos = e.getComponent<CTransform>().GlobalTranslation;
					for (size_t i = 0; i < vertices.size(); i++)
					{
						Vec2 rotatedPoint = ePos + vertices[i].rotate(-1 * e.getComponent<CTransform>().GlobalRotation / DEG_PER_RAD);
						m_PhysicsPoly.setPoint(i, sf::Vector2f(rotatedPoint.x, rotatedPoint.y));
					}
					renderTexture.draw(m_PhysicsPoly);
				}
			}
			else
			{
				if (e.hasComponent<CSpriteRenderer>())
				{
					if (e.getComponent<CSpriteRenderer>().texture != 0)
					{
						m_Texture = AssetManager::GetAsset<Texture>(e.getComponent<CSpriteRenderer>().texture)->GetSFMLTexture();
						sf::Sprite sprite = sf::Sprite(m_Texture);
						sprite.setOrigin(m_Texture.getSize().x / 2.0f, m_Texture.getSize().y / 2.0f);
						sprite.setPosition(e.getComponent<CTransform>().GlobalTranslation.x, e.getComponent<CTransform>().GlobalTranslation.y);
						sprite.setRotation(-1 * e.getComponent<CTransform>().GlobalRotation);
						renderTexture.draw(sprite);
					}
				}
				else if (e.hasComponent<CCircle>())
				{
					m_CircleShape.setPointCount(30);
					m_CircleShape.setRadius(e.getComponent<CCircle>().radius);
					m_CircleShape.setOrigin(e.getComponent<CCircle>().radius, e.getComponent<CCircle>().radius);
					m_CircleShape.setPosition(e.getComponent<CTransform>().GlobalTranslation.x, e.getComponent<CTransform>().GlobalTranslation.y);
					m_CircleShape.setFillColor(e.getComponent<CCircle>().color);
					renderTexture.draw(m_CircleShape);
				}
				else if (e.hasComponent<CRectangle>())
				{
					m_RectangleShape.setSize(sf::Vector2f(e.getComponent<CRectangle>().size.x, e.getComponent<CRectangle>().size.y));
					m_RectangleShape.setOrigin(e.getComponent<CRectangle>().size.x / 2, e.getComponent<CRectangle>().size.y / 2);
					m_RectangleShape.setRotation(-1 * e.getComponent<CTransform>().GlobalRotation);
					m_RectangleShape.setPosition(e.getComponent<CTransform>().GlobalTranslation.x, e.getComponent<CTransform>().GlobalTranslation.y);
					m_RectangleShape.setFillColor(e.getComponent<CRectangle>().color);
					m_RectangleShape.setOutlineColor(sf::Color::Transparent);
					renderTexture.draw(m_RectangleShape);
				}
				else if (e.hasComponent<CPolygon>())
				{
					m_CircleShape.setPointCount(e.getComponent<CPolygon>().sides);
					m_CircleShape.setRadius(e.getComponent<CPolygon>().size);
					m_CircleShape.setOrigin(e.getComponent<CPolygon>().size, e.getComponent<CPolygon>().size);
					m_CircleShape.setPosition(e.getComponent<CTransform>().GlobalTranslation.x, e.getComponent<CTransform>().GlobalTranslation.y);
					m_CircleShape.setRotation(-1 * e.getComponent<CTransform>().GlobalRotation);
					m_CircleShape.setFillColor(e.getComponent<CPolygon>().color);
					m_CircleShape.setOutlineColor(e.getComponent<CPolygon>().color);
					renderTexture.draw(m_CircleShape);
				}
			}
		
			if (e.hasComponent<CJoint>())
			{
				if (!m_IsRunning)
				{
					//TODO:  only if entity is selected
					/*m_CircleShape.setPointCount(30);
					m_CircleShape.setRadius(30.0f);
					m_CircleShape.setOrigin(30.0f, 30.0f);
					m_CircleShape.setPosition(e.getComponent<CTransform>().pos.x + e.getComponent<CJoint>().anchorPos.x, e.getComponent<CTransform>().pos.y + e.getComponent<CJoint>().anchorPos.y);
					m_CircleShape.setFillColor(sf::Color(221, 255, 221, 120));
					m_CircleShape.setOutlineColor(sf::Color(221, 255, 221, 255));
					renderTexture.draw(m_CircleShape);*/

					auto& jointComponent = e.getComponent<CJoint>();
					if (!IsEntityUUIDValid(jointComponent.entity2Id))
					{
						continue;
					}
					auto entity2 = GetEntityByUUID(jointComponent.entity2Id);
					if (!(e.hasComponent<CRigidBody>() && entity2.hasComponent<CRigidBody>()))
					{
						continue;
					}

				
					Vec2 p1 = e.getComponent<CTransform>().GlobalTranslation;
					Vec2 p2 = entity2.getComponent<CTransform>().GlobalTranslation;
					Vec2 anchorPos = p1 + e.getComponent<CJoint>().anchorPos;
					sf::VertexArray lines(sf::Lines, 4);
					lines[0].position = sf::Vector2f(p1.x, p1.y);
					lines[0].color = sf::Color(128, 128, 204);

					lines[1].position = sf::Vector2f(anchorPos.x, anchorPos.y);
					lines[1].color = sf::Color(128, 128, 204);

					// Line 2
					lines[2].position = sf::Vector2f(p2.x, p2.y);
					lines[2].color = sf::Color(128, 128, 204);

					lines[3].position = sf::Vector2f(anchorPos.x, anchorPos.y);
					lines[3].color = sf::Color(128, 128, 204);
					renderTexture.draw(lines);
				}
				else
				{
					auto& jointComponent = e.getComponent<CJoint>();
					if (!IsEntityUUIDValid(jointComponent.entity2Id))
					{
						continue;
					}
					auto entity2 = GetEntityByUUID(jointComponent.entity2Id);
					if (!(e.hasComponent<CRigidBody>() && entity2.hasComponent<CRigidBody>()))
					{
						continue;
					}

					if (e.getComponent<CJoint>().runtimeJoint)
					{
						PhysicsHingeJoint* joint = (PhysicsHingeJoint*)e.getComponent<CJoint>().runtimeJoint;

						Vec2 p1 = e.getComponent<CTransform>().GlobalTranslation;
						Vec2 p2 = entity2.getComponent<CTransform>().GlobalTranslation;
						Mat22 R1(e.getComponent<CTransform>().GlobalRotation / DEG_PER_RAD);
						Mat22 R2(entity2.getComponent<CTransform>().GlobalRotation / DEG_PER_RAD);

						Vec2 rotatedAnchorPos1 = R1 * joint->m_localAnchor1;
						Vec2 anchorPos1 = p1 + Vec2(rotatedAnchorPos1.x * PPM, -rotatedAnchorPos1.y * PPM);
						Vec2 rotatedAnchorPos2 = R2 * joint->m_localAnchor2;
						Vec2 anchorPos2 = p2 + Vec2(rotatedAnchorPos2.x * PPM, -rotatedAnchorPos2.y * PPM);

						sf::VertexArray lines(sf::Lines, 4);
						lines[0].position = sf::Vector2f(p1.x, p1.y);
						lines[0].color = sf::Color(128, 128, 204);

						lines[1].position = sf::Vector2f(anchorPos1.x, anchorPos1.y);
						lines[1].color = sf::Color(128, 128, 204);

						// Line 2
						lines[2].position = sf::Vector2f(p2.x, p2.y);
						lines[2].color = sf::Color(128, 128, 204);

						lines[3].position = sf::Vector2f(anchorPos2.x, anchorPos2.y);
						lines[3].color = sf::Color(128, 128, 204);
						renderTexture.draw(lines);
					}
				}
			}
		}

	if (m_IsRunning)
	{
		for (auto p : m_contactPoints)
		{
			m_CircleShape.setPointCount(30);
			m_CircleShape.setRadius(3.0f);
			m_CircleShape.setOrigin(3.0f, 3.0f);
			m_CircleShape.setPosition(p.x * PPM, -1 * p.y * PPM);
			m_CircleShape.setFillColor(sf::Color(255, 0, 0));
			m_CircleShape.setOutlineColor(sf::Color(255, 0, 0));
			renderTexture.draw(m_CircleShape);
		}
	}

	// Camera Editor Rendering (TODO: should be done in editor layer...
	if (!m_IsRunning && mainCamera)
	{
		g_cameraIconSprite.setOrigin(g_cameraIconTexture.getSize().x / 2.0f, g_cameraIconTexture.getSize().y / 2.0f);
		g_cameraIconSprite.setPosition(cameraTransform.GlobalTranslation.x, cameraTransform.GlobalTranslation.y);
		renderTexture.draw(g_cameraIconSprite);

		m_RectangleShape.setSize(sf::Vector2f(mainCamera->size.x, mainCamera->size.y));
		m_RectangleShape.setOrigin(mainCamera->size.x / 2, mainCamera->size.y / 2);
		m_RectangleShape.setPosition(cameraTransform.GlobalTranslation.x, cameraTransform.GlobalTranslation.y);
		m_RectangleShape.setFillColor(sf::Color::Transparent);
		m_RectangleShape.setOutlineColor(sf::Color::White);
		m_RectangleShape.setOutlineThickness(3.0f);
		renderTexture.draw(m_RectangleShape);
	}
	

#endif
	
}
