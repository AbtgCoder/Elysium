#include "Scene.h"

#include "Asset/AssetManager.h"
#include "Core/Texture.h"

#include "Scripts/RotateEntity.h"

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
	m_CircleShape.setOutlineColor(sf::Color::White);
	m_CircleShape.setOutlineThickness(1);
	m_CircleShape.setPointCount(30);

	m_RectangleShape.setFillColor(sf::Color::Transparent);
	m_RectangleShape.setOutlineColor(sf::Color::White);
	m_RectangleShape.setOutlineThickness(1);

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

		if (e.hasComponent<CPhysicsMaterial>())
		{
			runtimeEntity.addComponent<CPhysicsMaterial>(e.getComponent<CPhysicsMaterial>());
		}

		if (e.hasComponent<CNativeScriptComponent>())
		{
			runtimeEntity.addComponent<CNativeScriptComponent>().Bind<RotateEntity>();
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
	Vec2 ePos = e.getComponent<CTransform>().pos;
	if (pos.x > ePos.x - s.x / 2 &&
		pos.x < ePos.x + s.x / 2 &&
		pos.y > ePos.y - s.y / 2 &&
		pos.y < ePos.y + s.y / 2)
	{
		return true;
	}
	return false;
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
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CJoint>())
		{
			auto& joint = e.getComponent<CJoint>();
			if (joint.entity2Id > -1) //TODO: check if both entity id's are valid ??, and first id should be equal to entitie's id
			{
				std::cout << "joint info: " << joint.entity1Id << " " << joint.entity2Id << " " << joint.anchorPos << "\n";
			}
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
}

void Scene::OnUpdateRuntime(sf::RenderTexture& renderTexture, float dt)
{
	if (!m_IsPaused || m_StepFrames-- > 0)
	{
		auto runtimeEntities = m_entityManager.GetEntities();

		// Update scripts
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
			// Movement
			for (auto e : runtimeEntities)
			{
				// do at RuntimeStart
				float mass = 1.0f;
				float restitution = 1.0f;
				if (e.hasComponent<CPhysicsMaterial>())
				{
					mass = e.getComponent<CPhysicsMaterial>().mass;
					restitution = e.getComponent<CPhysicsMaterial>().restitutionCoefficient;
				}

				Vec2 v0 = e.getComponent<CTransform>().velocity;
				Vec2 r0 = e.getComponent<CTransform>().pos;

				std::vector<Vec2> velocities(m_velocityIterations + 1);
				std::vector<Vec2> positions(m_positionIterations + 1);
				std::vector<float> angularVelocities(m_velocityIterations + 1);
				std::vector<float> angles(m_positionIterations + 1);


				velocities[0] = v0;
				positions[0] = r0;
				angularVelocities[0] = e.getComponent<CTransform>().angularVelocity;
				angles[0] = e.getComponent<CTransform>().angle;

				Vec2 pointOfApplication = r0 + Vec2(1.0f, 1.0f);
				Vec2 r = pointOfApplication - r0; 
				float torque = r.cross(m_externalForce); // r X F
				float momentOfInertia = 1.0f; // calculate at OnRuntimeStart about COM (only for rigidbodys ??)
				if (e.hasComponent<CBoundingBox>())
				{
					momentOfInertia = 1 / 6 * mass * e.getComponent<CBoundingBox>().size.x * e.getComponent<CBoundingBox>().size.x;
				}

				float h = dt;

				// angular velocity integration
				for (size_t i = 0; i < m_velocityIterations; i++)
				{
					angularVelocities[i + 1] = angularVelocities[i] + (torque / momentOfInertia) * h;
				}
				// angle integration
				for (size_t i = 0; i < m_positionIterations; i++)
				{
					angles[i + 1] = angles[i] + angularVelocities[i] * h;
				}
				// velocity integration
				for (size_t i = 0; i < m_velocityIterations; i++)
				{
					velocities[i + 1] = velocities[i] + (m_gravity + (m_externalForce / mass)) * h;
				}
				// position integration
				for (size_t i = 0; i < m_positionIterations; i++)
				{
					positions[i + 1] = positions[i] + velocities[i] * h;
				}

				e.getComponent<CTransform>().angularVelocity = angularVelocities[m_velocityIterations];
				e.getComponent<CTransform>().angle = angles[m_positionIterations];

				e.getComponent<CTransform>().velocity = velocities[m_velocityIterations];

				e.getComponent<CTransform>().prevPos = e.getComponent<CTransform>().pos;
				
				e.getComponent<CTransform>().pos = positions[m_positionIterations];

				// Rectangular Bounds
				if ((e.getComponent<CTransform>().pos.y > 800 || e.getComponent<CTransform>().pos.y < 200))
				{
					e.getComponent<CTransform>().velocity.y *= -restitution;
				}
				if (e.getComponent<CTransform>().pos.x > 1200 || e.getComponent<CTransform>().pos.x < -200)
				{
					e.getComponent<CTransform>().velocity.x *= -restitution;
				}
				
			}

			// Collision Detection and Resolution
			if (m_KDTreeBroadPhaseCollision)
			{
				// Broadphase collision detection
				KDTreeNode* rootNode = new KDTreeNode();
				for (auto e : runtimeEntities)
				{
					rootNode->entities.push_back(e);
				}
				makeKDTree(rootNode, 0);

				// Narrowphase collision detection
				Physics::NarrowPhaseCollision(rootNode);

				delete rootNode;
			}
			else
			{
				for (size_t i = 0; i < runtimeEntities.size(); i++)
				{
					for (size_t j = i + 1; j < runtimeEntities.size(); j++)
					{
						if (runtimeEntities[i].hasComponent<CPolygonCollider>() && runtimeEntities[j].hasComponent<CPolygonCollider>())
						{
							if (Physics::SAT(runtimeEntities[i], runtimeEntities[j]))
							{
								std::cout << runtimeEntities[i].getComponent<CTag>().tag << " collided with " << runtimeEntities[j].getComponent<CTag>().tag << "\n";
							}
						}
						else if (runtimeEntities[i].hasComponent<CCircleCollider>() && runtimeEntities[j].hasComponent<CCircleCollider>())
						{
							if (Physics::CircleCircleCollision(runtimeEntities[i], runtimeEntities[j]))
							{
								std::cout << runtimeEntities[i].getComponent<CTag>().tag << " collided with " << runtimeEntities[j].getComponent<CTag>().tag << "\n";
							}
						}
						else if (runtimeEntities[i].hasComponent<CBoundingBox>() && runtimeEntities[j].hasComponent<CBoundingBox>())
						{
							if (Physics::AABBElasticCollision(runtimeEntities[i], runtimeEntities[j]))
							{
								std::cout << runtimeEntities[i].getComponent<CTag>().tag << " collided with " << runtimeEntities[j].getComponent<CTag>().tag << "\n";
							}
						}
					}
				}
			}
		}
	}


	// Rendering
	RenderScene(renderTexture);
}

void Scene::OnUpdateEditor(sf::RenderTexture& renderTexture)
{
	// stuff
	RenderScene(renderTexture);
}

void Scene::Step(int frames)
{
	m_StepFrames = frames;
}

void Scene::RenderScene(sf::RenderTexture& renderTexture)
{
	//renderTexture.clear(sf::Color::Blue);
	renderTexture.clear();

	for (auto e : m_entityManager.GetEntities())
	{
		if (m_drawPhysicsColliders)
		{
			if (e.hasComponent<CBoundingBox>())
			{
				Vec2 rectSize = e.getComponent<CBoundingBox>().size;
				m_PhysicsRect.setSize(sf::Vector2f(rectSize.x, rectSize.y));
				m_PhysicsRect.setOrigin(rectSize.x / 2, rectSize.y / 2);
				m_PhysicsRect.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
				renderTexture.draw(m_PhysicsRect);
			}
			if (e.hasComponent<CPolygonCollider>())
			{
				std::vector<Vec2> vertices = e.getComponent<CPolygonCollider>().colliderVertices;
				m_PhysicsPoly.setPointCount(vertices.size());
				Vec2 ePos = e.getComponent<CTransform>().pos;
				sf::Texture tex = AssetManager::GetAsset<Texture>(e.getComponent<CSpriteRenderer>().texture)->GetSFMLTexture();
				//Vec2 eSize(tex.getSize().x, tex.getSize().y);
				Vec2 eSize = e.getComponent<CPolygonCollider>().size;
				for (size_t i = 0; i < vertices.size(); i++)
				{
					m_PhysicsPoly.setPoint(i, sf::Vector2f(ePos.x - eSize.x/2 + vertices[i].x, ePos.y + eSize.y/2 - vertices[i].y));
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
					sprite.setOrigin(tex.getSize().x / 2, tex.getSize().y / 2);
					sprite.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
					renderTexture.draw(sprite);
				}
			}
			else if (e.hasComponent<CCircle>())
			{
				m_CircleShape.setRadius(e.getComponent<CCircle>().radius);
				m_CircleShape.setOrigin(e.getComponent<CCircle>().radius, e.getComponent<CCircle>().radius);
				m_CircleShape.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
				renderTexture.draw(m_CircleShape);
			}
			else if (e.hasComponent<CRectangle>())
			{
				m_RectangleShape.setSize(sf::Vector2f(e.getComponent<CRectangle>().size.x, e.getComponent<CRectangle>().size.y));
				m_RectangleShape.setOrigin(e.getComponent<CRectangle>().size.x / 2, e.getComponent<CRectangle>().size.y / 2);
				m_RectangleShape.setRotation(-1*e.getComponent<CTransform>().angle);
				m_RectangleShape.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
				renderTexture.draw(m_RectangleShape);
			}

			//TODO:  only if entity is selected
		/*	if (e.hasComponent<CJoint>())
			{
				m_CircleShape.setRadius(30.0f);
				m_CircleShape.setOrigin(30.0f, 30.0f);
				m_CircleShape.setPosition(e.getComponent<CTransform>().pos.x + e.getComponent<CJoint>().anchorPos.x, e.getComponent<CTransform>().pos.y + e.getComponent<CJoint>().anchorPos.y);
				m_CircleShape.setFillColor(sf::Color(221, 255, 221, 120));
				m_CircleShape.setOutlineColor(sf::Color(221, 255, 221, 255));
				renderTexture.draw(m_CircleShape);
			}*/
		}

	}

}
