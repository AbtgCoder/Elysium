#include "core/Log.h"

#include "Scene.h"

#include "Asset/AssetManager.h"
#include "Core/Texture.h"
#include "Asset/TextureImporter.h"

#include "Scripts/RotateEntity.h"


// Pixels per meter. Box2D uses metric units, so we need to define a conversion
#define PPM 30.0F
// SFML uses degrees for angles while Box2D uses radians
#define DEG_PER_RAD 57.2957795F


sf::Texture g_cameraIconTexture = TextureImporter::LoadTexture("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/CameraIcon2.png")->GetSFMLTexture();
sf::Sprite g_cameraIconSprite = sf::Sprite(g_cameraIconTexture);

Scene::Scene()
	: Scene("Untitled")
{
}

Scene::Scene(const std::string& name)
	: m_Name(name)
{
	m_PhysicsRect.setFillColor(sf::Color::Transparent);
	m_PhysicsRect.setOutlineColor(sf::Color::White);
	m_PhysicsRect.setOutlineThickness(1);

	m_PhysicsPoly.setFillColor(sf::Color::Transparent);
	m_PhysicsPoly.setOutlineColor(sf::Color::White);
	m_PhysicsPoly.setOutlineThickness(1);

	m_CircleShape.setFillColor(sf::Color::Transparent);
	//m_CircleShape.setOutlineColor(sf::Color::White);
	//m_CircleShape.setOutlineThickness(1);
	m_CircleShape.setPointCount(30);

	m_RectangleShape.setFillColor(sf::Color::Transparent);
	/*m_RectangleShape.setOutlineColor(sf::Color::White);
	m_RectangleShape.setOutlineThickness(1);*/

}

Scene::~Scene()
{
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
		if (e.hasComponent<CSpriteRenderer>())
		{
			runtimeEntity.addComponent<CSpriteRenderer>(e.getComponent<CSpriteRenderer>());
		}
		else if (e.hasComponent<CCircle>())
		{
			runtimeEntity.addComponent<CCircle>(e.getComponent<CCircle>());
		}
		else if (e.hasComponent<CRectangle>())
		{
			runtimeEntity.addComponent<CRectangle>(e.getComponent<CRectangle>());
		}
		else if (e.hasComponent<CPolygon>())
		{
			runtimeEntity.addComponent<CPolygon>(e.getComponent<CPolygon>());
		}

		if (e.hasComponent<CCircleCollider>())
		{
			runtimeEntity.addComponent<CCircleCollider>(e.getComponent<CCircleCollider>());
		}
		else if (e.hasComponent<CBoundingBox>())
		{
			runtimeEntity.addComponent<CBoundingBox>(e.getComponent<CBoundingBox>());
		}
		else if (e.hasComponent<CPolygonCollider>())
		{
			runtimeEntity.addComponent<CPolygonCollider>(e.getComponent<CPolygonCollider>());
		}
		if (e.hasComponent<CRigidBody>())
		{
			runtimeEntity.addComponent<CRigidBody>(e.getComponent<CRigidBody>());
		}
		if (e.hasComponent<CPhysicsMaterial>())
		{
			runtimeEntity.addComponent<CPhysicsMaterial>(e.getComponent<CPhysicsMaterial>());
		}
		if (e.hasComponent<CJoint>())
		{
			runtimeEntity.addComponent<CJoint>(e.getComponent<CJoint>());
		}

		if (e.hasComponent<CNativeScriptComponent>())
		{
			runtimeEntity.addComponent<CNativeScriptComponent>().Bind<RotateEntity>();
		}

		if (e.hasComponent<CCamera>())
		{
			runtimeEntity.addComponent<CCamera>(e.getComponent<CCamera>());
		}
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
	auto& tag = e.addComponent<CTag>();
	tag.tag = name.empty() ? "Entity" : name;
	m_entityManager.update();
	return e;
}

Entity Scene::AddEntityWithSprite(Vec2 pos, AssetHandle textureHandle)
{
	// asset, asset type as texture
	auto entity = m_entityManager.addEntity();
	entity.addComponent<CId>(Elysium::UUID());
	entity.addComponent<CTag>("Tile");
	entity.addComponent<CTransform>(pos);
	entity.addComponent<CSpriteRenderer>();
	entity.getComponent<CSpriteRenderer>().texture = textureHandle;
	return entity;
}

//TODO: improve "Duplication" functionality
Entity Scene::DuplicateEntity(Entity e)
{
	auto duplicateEntity = AddEntity(e.getComponent<CTag>().tag);
	duplicateEntity.addComponent<CTransform>(e.getComponent<CTransform>());
	if (e.hasComponent<CSpriteRenderer>())
	{
		duplicateEntity.addComponent<CSpriteRenderer>(e.getComponent<CSpriteRenderer>());
	}
	else if (e.hasComponent<CCircle>())
	{
		duplicateEntity.addComponent<CCircle>(e.getComponent<CCircle>());
	}
	else if (e.hasComponent<CRectangle>())
	{
		duplicateEntity.addComponent<CRectangle>(e.getComponent<CRectangle>());
	}
	else if (e.hasComponent<CPolygon>())
	{
		duplicateEntity.addComponent<CPolygon>(e.getComponent<CPolygon>());
	}

	if (e.hasComponent<CCircleCollider>())
	{
		duplicateEntity.addComponent<CCircleCollider>(e.getComponent<CCircleCollider>());
	}
	else if (e.hasComponent<CBoundingBox>())
	{
		duplicateEntity.addComponent<CBoundingBox>(e.getComponent<CBoundingBox>());
	}
	else if (e.hasComponent<CPolygonCollider>())
	{
		duplicateEntity.addComponent<CPolygonCollider>(e.getComponent<CPolygonCollider>());
	}

	if (e.hasComponent<CRigidBody>())
	{
		duplicateEntity.addComponent<CRigidBody>(e.getComponent<CRigidBody>());
	}
	if (e.hasComponent<CPhysicsMaterial>())
	{
		duplicateEntity.addComponent<CPhysicsMaterial>(e.getComponent<CPhysicsMaterial>());
	}
	if (e.hasComponent<CJoint>())
	{
		duplicateEntity.addComponent<CJoint>(e.getComponent<CJoint>());
	}

	if (e.hasComponent<CNativeScriptComponent>())
	{
		duplicateEntity.addComponent<CNativeScriptComponent>().Bind<RotateEntity>();
	}
	m_entityManager.update();
	return duplicateEntity;
}

static bool IsInside(Vec2 pos, Entity e)
{
	sf::Vector2u s;
	if (e.hasComponent<CSpriteRenderer>())
	{
		s = AssetManager::GetAsset<Texture>(e.getComponent<CSpriteRenderer>().texture)->GetSFMLTexture().getSize();
	}
	else if (e.hasComponent<CCircle>())
	{
		s.x = 2 * e.getComponent<CCircle>().radius;
		s.y = 2 * e.getComponent<CCircle>().radius;
	}
	else if (e.hasComponent<CRectangle>())
	{
		s.x = e.getComponent<CRectangle>().size.x;
		s.y = e.getComponent<CRectangle>().size.y;
	}
	else if (e.hasComponent<CCamera>())
	{
		s = g_cameraIconTexture.getSize() + sf::Vector2u(10.0f, 10.0f);
	}
	Vec2 ePos = e.getComponent<CTransform>().pos;
	if (pos.x > ePos.x - s.x / 2 &&
		pos.x < ePos.x + s.x / 2 &&
		pos.y > ePos.y - s.y / 2 &&
		pos.y < ePos.y + s.y / 2)
	{
		return true;
	}
	return false;
}

Entity Scene::GetEntityIfClicked(Vec2 mousePos)
{
	for (auto entity : m_entityManager.GetEntities())
	{
		if (IsInside(mousePos, entity))
		{
			return entity;
		}
	}

	return {};
}

Entity Scene::GetEntityByUUID(Elysium::UUID id)
{
	//TODO: assert valid id probably
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.getComponent<CId>().id == id)
		{
			return e;
		}
	}
}

void Scene::DestroyEntity(Entity entity)
{
	entity.destroy();
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

void Scene::OnRuntimeStart()
{
	ESM_INFO("Starting Runtime");

	m_IsRunning = true;

	// Instantiate script
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CNativeScriptComponent>())
		{
			auto& nsc = e.getComponent<CNativeScriptComponent>();
			nsc.instance = nsc.InstantiateScript();
			nsc.instance->m_Entity = e;
			nsc.instance->OnCreate();
		}
	}

	// Physics world initialization
	m_PhysicsWorld = new PhysicsWorld({0.0f, -9.8f}, 10);
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CRigidBody>())
		{
			auto& transform = e.getComponent<CTransform>();
			auto& rb2d = e.getComponent<CRigidBody>();

			PhysicsBody* body = new PhysicsBody();
			body->m_position = { transform.pos.x / PPM, -1 * transform.pos.y / PPM };
			body->m_rotation = transform.angle / DEG_PER_RAD;
			body->m_velocity = transform.velocity;
			body->m_angularVelocity = transform.angularVelocity;
			body->m_type = rb2d.Type == CRigidBody::BodyType::Static ? PhysicsBodyType::staticBody : PhysicsBodyType::dynamicBody;
			rb2d.runtimeBody = body;

			if (e.hasComponent<CBoundingBox>())
			{
				auto& bb2d = e.getComponent<CBoundingBox>();
				PhysicsPolygonShape* boxShape = new PhysicsPolygonShape();
				boxShape->SetAsBox(bb2d.halfSize.x * transform.scale.x / PPM, bb2d.halfSize.y * transform.scale.y / PPM, bb2d.offset / PPM, 0.0f);
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
					points.push_back({ point.x / PPM, -1 * point.y / PPM });
				}
				PhysicsPolygonShape* polyShape = new PhysicsPolygonShape();
				polyShape->Set(points);
				body->m_shape = polyShape;
			}
			else if (e.hasComponent<CCircleCollider>())
			{
				PhysicsCircleShape* circleShape = new PhysicsCircleShape();
				circleShape->m_p.Set(0.0f, 0.0f); // TODO: should be offset
				circleShape->m_radius = e.getComponent<CCircleCollider>().radius * e.getComponent<CTransform>().scale.x / PPM;
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
			Vec2 anchorWorldPos = jointComponent.anchorPos + e.getComponent<CTransform>().pos;
			PhysicsHingeJoint* joint = new PhysicsHingeJoint();
			joint->Set(body1, body2, Vec2(anchorWorldPos.x / PPM, - anchorWorldPos.y / PPM));
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

	// Destroy script
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CNativeScriptComponent>())
		{
			auto& nsc = e.getComponent<CNativeScriptComponent>();
			nsc.instance->OnDestroy();
			nsc.DestroyScript(&nsc);
		}
	}

	// Physics world deletion
	delete m_PhysicsWorld;
	if (m_bomb)
		m_bomb.destroy();
}

void Scene::OnUpdateRuntime(sf::RenderTexture& renderTexture, float dt)
{
	if (!m_IsPaused || m_StepFrames-- > 0)
	{
		m_contactPoints.clear();

		auto runtimeEntities = m_entityManager.GetEntities();

		// Update Native Scripts
		{
			for (auto e : m_entityManager.GetEntities())
			{
				if (e.hasComponent<CNativeScriptComponent>())
				{
					auto& nsc = e.getComponent<CNativeScriptComponent>();
					if (nsc.instance)
						nsc.instance->OnUpdate(dt);
				}
			}
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
						PhysicsBody* body = (PhysicsBody*)rb2d.runtimeBody;
						e.getComponent<CTransform>().velocity = body->m_velocity;
						e.getComponent<CTransform>().pos = { body->m_position.x * PPM, -1 * body->m_position.y * PPM };
						e.getComponent<CTransform>().angle = body->m_rotation * DEG_PER_RAD;
						e.getComponent<CTransform>().angularVelocity = body->m_angularVelocity;
					}
				}
			}

		}
	}


	// Rendering
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

	if (mainCamera)
	{
		m_cameraView.setSize(mainCamera->size.x, mainCamera->size.y);
		m_cameraView.setCenter(cameraTransform.pos.x, cameraTransform.pos.y);
		m_cameraView.zoom(mainCamera->zoom);
		m_cameraView.setRotation(cameraTransform.angle);
		renderTexture.setView(m_cameraView);
		renderTexture.clear(mainCamera->backgroundColor);

		RenderScene(renderTexture);

	}
	else
	{
		//TODO: editor log: there is no primary camera in scene
	}

	
}

void Scene::OnUpdateEditor(sf::RenderTexture& renderTexture)
{
	// stuff
	RenderScene(renderTexture);
}

void Scene::LaunchBomb(sf::RenderTexture& renderTexture)
{
	if (!m_bomb)
	{
		m_bomb = AddEntity("runtime_bomb");
		auto& rb2d = m_bomb.addComponent<CRigidBody>();
		auto& transform = m_bomb.getComponent<CTransform>();
		int xmin = 900, xmax = 1500, ymin = 100, ymax = 400;
		transform.pos = Vec2(rand() % (xmax - xmin + 1) + xmin, rand() % (ymax - ymin + 1) + ymin);
		transform.angle = rand() % (360 + 1);
		int vmin = -6, vmax = 6;
		transform.velocity = Vec2(rand() % (vmax - vmin + 1) +vmin, rand() % (vmax - vmin + 1) + vmin);
		transform.angularVelocity = rand() % (vmax*2 - vmin*2 + 1) + vmin*2;
		auto& rectangle = m_bomb.addComponent<CRectangle>();
		rectangle.color = sf::Color(0, 255, 0, 255);
		auto& bb2d = m_bomb.addComponent<CBoundingBox>(rectangle.size);
		
		PhysicsBody* body = new PhysicsBody();
		body->m_position = { transform.pos.x / PPM, -1 * transform.pos.y / PPM };
		body->m_rotation = transform.angle / DEG_PER_RAD;
		body->m_velocity = transform.velocity;
		body->m_angularVelocity = transform.angularVelocity;
		body->m_type = PhysicsBodyType::dynamicBody;
		rb2d.runtimeBody = body;

		PhysicsPolygonShape* boxShape = new PhysicsPolygonShape();
		boxShape->SetAsBox(bb2d.halfSize.x * transform.scale.x / PPM, bb2d.halfSize.y * transform.scale.y / PPM, bb2d.offset / PPM, 0.0f);
		body->m_shape = boxShape;
		body->ResetMassData(5.7f);

		m_PhysicsWorld->AddBody(body);
		return;
	}
	auto& transform = m_bomb.getComponent<CTransform>();
	int xmin = 100, xmax = 1000, ymin = 200, ymax = 800;
	transform.pos = Vec2(rand() % (xmax - xmin + 1) + xmin, rand() % (ymax - ymin + 1) + ymin);
	transform.angle = rand() % (360 + 1);
	int vmin = -6, vmax = 6;
	transform.velocity = Vec2(rand() % (vmax - vmin + 1) + vmin, rand() % (vmax - vmin + 1) + vmin);
	transform.angularVelocity = rand() % (vmax * 2 - vmin * 2 + 1) + vmin * 2;
	auto& rb2d = m_bomb.getComponent<CRigidBody>();
	PhysicsBody* body = (PhysicsBody*)rb2d.runtimeBody;
	body->m_position = { transform.pos.x / PPM, -1 * transform.pos.y / PPM };
	body->m_rotation = transform.angle / DEG_PER_RAD;
	body->m_velocity = transform.velocity;
	body->m_angularVelocity = transform.angularVelocity;
	
}

void Scene::Step(int frames)
{
	m_StepFrames = frames;
}

void Scene::RenderScene(sf::RenderTexture& renderTexture)
{

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
					m_CircleShape.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
					m_CircleShape.setRotation(-1 * e.getComponent<CTransform>().angle);
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
					m_PhysicsRect.setRotation(-1 * e.getComponent<CTransform>().angle);
					m_PhysicsRect.setPosition(e.getComponent<CTransform>().pos.x + e.getComponent<CBoundingBox>().offset.x, e.getComponent<CTransform>().pos.y + e.getComponent<CBoundingBox>().offset.y);
					m_PhysicsRect.setFillColor(sf::Color::Transparent);
					m_PhysicsRect.setOutlineColor(e.getComponent<CRectangle>().color);
					renderTexture.draw(m_PhysicsRect);
				}
				else if (e.hasComponent<CPolygonCollider>())
				{
					std::vector<Vec2> vertices = e.getComponent<CPolygonCollider>().colliderVertices;
					m_PhysicsPoly.setPointCount(vertices.size());
					Vec2 ePos = e.getComponent<CTransform>().pos;
					for (size_t i = 0; i < vertices.size(); i++)
					{
						Vec2 rotatedPoint = ePos + vertices[i].rotate(-1 * e.getComponent<CTransform>().angle / DEG_PER_RAD);
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
						// highly inefficient drawing
						sf::Texture tex = AssetManager::GetAsset<Texture>(e.getComponent<CSpriteRenderer>().texture)->GetSFMLTexture();
						sf::Sprite sprite = sf::Sprite(tex);
						sprite.setOrigin(tex.getSize().x / 2.0f, tex.getSize().y / 2.0f);
						sprite.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
						sprite.setRotation(-1 * e.getComponent<CTransform>().angle);
						renderTexture.draw(sprite);
					}
				}
				else if (e.hasComponent<CCircle>())
				{
					m_CircleShape.setPointCount(30);
					m_CircleShape.setRadius(e.getComponent<CCircle>().radius);
					m_CircleShape.setOrigin(e.getComponent<CCircle>().radius, e.getComponent<CCircle>().radius);
					m_CircleShape.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
					m_CircleShape.setFillColor(e.getComponent<CCircle>().color);
					renderTexture.draw(m_CircleShape);
				}
				else if (e.hasComponent<CRectangle>())
				{
					m_RectangleShape.setSize(sf::Vector2f(e.getComponent<CRectangle>().size.x, e.getComponent<CRectangle>().size.y));
					m_RectangleShape.setOrigin(e.getComponent<CRectangle>().size.x / 2, e.getComponent<CRectangle>().size.y / 2);
					m_RectangleShape.setRotation(-1 * e.getComponent<CTransform>().angle);
					m_RectangleShape.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
					m_RectangleShape.setFillColor(e.getComponent<CRectangle>().color);
					m_RectangleShape.setOutlineColor(sf::Color::Transparent);
					renderTexture.draw(m_RectangleShape);
				}
				else if (e.hasComponent<CPolygon>())
				{
					m_CircleShape.setPointCount(e.getComponent<CPolygon>().sides);
					m_CircleShape.setRadius(e.getComponent<CPolygon>().size);
					m_CircleShape.setOrigin(e.getComponent<CPolygon>().size, e.getComponent<CPolygon>().size);
					m_CircleShape.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
					m_CircleShape.setRotation(-1 * e.getComponent<CTransform>().angle);
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

				
					Vec2 p1 = e.getComponent<CTransform>().pos;
					Vec2 p2 = entity2.getComponent<CTransform>().pos;
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

						Vec2 p1 = e.getComponent<CTransform>().pos;
						Vec2 p2 = entity2.getComponent<CTransform>().pos;
						Mat22 R1(e.getComponent<CTransform>().angle / DEG_PER_RAD);
						Mat22 R2(entity2.getComponent<CTransform>().angle / DEG_PER_RAD);

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

	// Camera Editor Rendering
	if (!m_IsRunning && mainCamera)
	{
		g_cameraIconSprite.setOrigin(g_cameraIconTexture.getSize().x / 2.0f, g_cameraIconTexture.getSize().y / 2.0f);
		g_cameraIconSprite.setPosition(cameraTransform.pos.x, cameraTransform.pos.y);
		renderTexture.draw(g_cameraIconSprite);

		m_RectangleShape.setSize(sf::Vector2f(mainCamera->size.x, mainCamera->size.y));
		m_RectangleShape.setOrigin(mainCamera->size.x / 2, mainCamera->size.y / 2);
		m_RectangleShape.setPosition(cameraTransform.pos.x, cameraTransform.pos.y);
		m_RectangleShape.setFillColor(sf::Color::Transparent);
		m_RectangleShape.setOutlineColor(sf::Color::White);
		m_RectangleShape.setOutlineThickness(3.0f);
		renderTexture.draw(m_RectangleShape);
	}
	
	
}
