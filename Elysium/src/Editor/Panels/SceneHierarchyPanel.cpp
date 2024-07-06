#include "SceneHierarchyPanel.h"

#include "Asset/AssetManager.h"
#include "core/Texture.h"
#include "Physics/graham_scan.h"

#include "ImGui/ImGuiHelper.h"

SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene>& Scene)
{
	SetScene(Scene);
}

void SceneHierarchyPanel::SetScene(const std::shared_ptr<Scene>& Scene)
{
	m_Scene = Scene;
	m_InspectedEntity = {};
}




template<typename T, typename UIFunction>
static void DrawComponentGUI(const std::string& name, Entity entity, UIFunction uiFunction)
{
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
	if (entity.hasComponent<T>())
	{
		auto& component = entity.getComponent<T>();
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();
		ImGui::SameLine(contentRegionAvailable.x - lineHeight);
		if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
		{
			ImGui::OpenPopup("ComponentSettings");
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				removeComponent = true;
			}
			ImGui::EndPopup();
		}

		if (open)
		{
			uiFunction(component);
			ImGui::TreePop();
		}

		if (removeComponent)
		{
			entity.removeComponent<T>();
		}
	}
}

std::vector<Vec2> generatePolygonColliderVertices(sf::Texture entityTex, Entity e)
{
	sf::Texture tex = entityTex; 
	sf::Image image = tex.copyToImage();
	sf::Vector2u imageSize = image.getSize();

	sf::Image paddedBinaryImage;
	paddedBinaryImage.create(imageSize.x + 2, imageSize.y + 2);

	for (int y = 0; y < imageSize.y + 2; ++y) {
		for (int x = 0; x < imageSize.x + 2; ++x) {
			if (y - 1 >= 0 && y - 1 < imageSize.y && x - 1 >= 0 && x - 1 < imageSize.x)
			{
				sf::Color pixelColor = image.getPixel(x - 1, y - 1);
				int grayscaleColor = static_cast<int>((pixelColor.r + pixelColor.g + pixelColor.b) / 3);
				if (grayscaleColor != 0)
				{
					paddedBinaryImage.setPixel(x, y, sf::Color::White);
				}
				else
				{
					paddedBinaryImage.setPixel(x, y, sf::Color::Black);
				}
			}
			else
			{
				paddedBinaryImage.setPixel(x, y, sf::Color::Black);
			}
		}
	}


	// boundaryPoints = countourTracing(paddedBinaryImage) TODO: moore neighborhood contour tracing ??
	std::vector<Vec2> boundaryPoints;
	for (uint32_t y = 0; y < imageSize.y; ++y)
	{
		for (uint32_t x = 0; x < imageSize.x; ++x)
		{
			if (paddedBinaryImage.getPixel(x, y) == sf::Color::White &&
				(paddedBinaryImage.getPixel(x - 1, y - 1) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x, y - 1) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x + 1, y - 1) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x - 1, y) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x + 1, y) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x - 1, y + 1) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x, y + 1) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x + 1, y + 1) == sf::Color::Black))
			{
				boundaryPoints.push_back(Vec2((float)x, (float)imageSize.y - y));
			}
		}
	}

	// TODO: reducing points ?? ramer-douglas-peucker algorithm

	// TODO: more algs :  jarvis march, chan's algorithm etc
	std::vector<Vec2> convexHull = grahamScan(boundaryPoints);

	//std::vector<Vec2> colliderVertices;
	//Vec2 ePos = e.getComponent<CTransform>().pos; 
	//Vec2 eSize(tex.getSize().x, tex.getSize().y);
	//for (auto p : convexHull)
	//{
	//	colliderVertices.push_back(Vec2(ePos.x - eSize.x / 2 + p.x, ePos.y + eSize.y / 2 - p.y));
	//}
	//return colliderVertices;
	return convexHull;
}

void SceneHierarchyPanel::OnImGuiRender()
{

	if (m_Scene)
	{
		m_Scene->m_entityManager.update(); // separate update function ??
		
		ImGui::Begin("Hierarchy");

		auto& sceneName = m_Scene->m_Name;
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strncpy_s(buffer, sizeof(buffer), sceneName.c_str(), sizeof(buffer));
		if (ImGui::InputText("##SceneName", buffer, sizeof(buffer)))
		{
			sceneName = std::string(buffer);
		}

		for (auto e : m_Scene->m_entityManager.GetEntities())
		{
			DrawEntityNode(e);
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_InspectedEntity = {};
		}

		// Right click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				auto entity = m_Scene->m_entityManager.addEntity();
				entity.addComponent<CTag>("Empty Entity");
				entity.addComponent<CTransform>();
			}
			if (ImGui::MenuItem("Create Circle Entity"))
			{
				auto entity = m_Scene->m_entityManager.addEntity();
				entity.addComponent<CTag>("Circle");
				entity.addComponent<CTransform>();
				entity.addComponent<CCircle>();
			}
			if (ImGui::MenuItem("Create Rectangle Entity"))
			{
				auto entity = m_Scene->m_entityManager.addEntity();
				entity.addComponent<CTag>("Rectangle");
				entity.addComponent<CTransform>();
				entity.addComponent<CRectangle>();
			}
			ImGui::EndPopup();
		}
		ImGui::End();
	}


	ImGui::Begin("Entity Inspector");
	if (m_InspectedEntity)
	{
		auto& tag = m_InspectedEntity.getComponent<CTag>().tag;
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));
		if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
		{
			tag = std::string(buffer);
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<CTransform>("Transform");
			if (m_InspectedEntity.hasComponent<CSpriteRenderer>())
			{
				if (m_InspectedEntity.getComponent<CSpriteRenderer>().texture != 0)
				{
					sf::Texture tex = AssetManager::GetAsset<Texture>(m_InspectedEntity.getComponent<CSpriteRenderer>().texture)->GetSFMLTexture();
					DisplayAddComponentEntry<CBoundingBox>("Box Collider 2D", Vec2(tex.getSize().x, tex.getSize().y));
					DisplayAddComponentEntry<CPolygonCollider>("Polygon Collider 2D", Vec2(tex.getSize().x, tex.getSize().y), generatePolygonColliderVertices(tex, m_InspectedEntity));
				}
			}
			else if (m_InspectedEntity.hasComponent<CCircle>())
			{
				DisplayAddComponentEntry<CCircleCollider>("Circle Collider 2D", m_InspectedEntity.getComponent<CCircle>().radius);
			}
			else if (m_InspectedEntity.hasComponent<CRectangle>())
			{
				DisplayAddComponentEntry<CBoundingBox>("Box Collider 2D", m_InspectedEntity.getComponent<CRectangle>().size);
			}
			else
			{
				DisplayAddComponentEntry<CCircle>("Circle Shape");
				DisplayAddComponentEntry<CRectangle>("Rectangle Shape");
				DisplayAddComponentEntry<CCircleCollider>("Circle Collider 2D");
				DisplayAddComponentEntry<CBoundingBox>("Box Collider 2D");
				DisplayAddComponentEntry<CPolygonCollider>("Polygon Collider 2D");
			}
			DisplayAddComponentEntry<CSpriteRenderer>("Sprite Renderer");
			DisplayAddComponentEntry<CPhysicsMaterial>("Physics Material");
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();


		DrawComponentGUI<CTransform>("Transform", m_InspectedEntity, [](auto& component)
			{
				DrawVec2Control("Position", component.pos, 0.0f, 80.0f);
				DrawVec2Control("Velocity", component.velocity, 0.0f, 80.0f);
				DrawVec2Control("Scale", component.scale, 0.0f, 80.0f);
				DrawFloatControl("Angle", component.angle, 0.0f, 360.0f);
				DrawFloatControl("Angular velocity", component.angularVelocity, -100.0f, 100.0f, 130.0f);
			});

		DrawComponentGUI<CCircle>("Circle Shape", m_InspectedEntity, [](auto& component) 
			{
				DrawFloatControl("Radius", component.radius, 0.0f, 200.0f);
			});

		DrawComponentGUI<CRectangle>("Rectangle Shape", m_InspectedEntity, [](auto& component)
			{
				DrawVec2Control("Size", component.size, 0.0f, 80.0f);
			});

		DrawComponentGUI<CCircleCollider>("Circle Collider 2D", m_InspectedEntity, [](auto& component)
			{
				DrawFloatControl("Radius", component.radius, 0.0f, 200.0f);
			});

		DrawComponentGUI<CPhysicsMaterial>("Physics Material", m_InspectedEntity, [](auto& component)
			{
				DrawFloatControl("Mass", component.mass, 1.0f, 100.0f);
				DrawFloatControl("Restitution", component.restitutionCoefficient, 0.0f, 1.0f);
			});

		DrawComponentGUI<CSpriteRenderer>("Sprite Renderer", m_InspectedEntity, [](auto& component)
			{
				std::string label = "None";
				bool isTextureValid = false;
				if (component.texture != 0)
				{
					if (AssetManager::IsAssetHandleValid(component.texture)
						&& AssetManager::GetAssetType(component.texture) == AssetType::Texture)
					{
						const std::filesystem::path& textureFilepath = Project::GetActive()->GetEditorAssetManager()->GetFilePath(component.texture);
						label = textureFilepath.filename().string();
						isTextureValid = true;
					}
					else
					{
						label = "Invalid";
					}
				}

				ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
				buttonLabelSize.x += 20.0f;
				float buttonLabelWidth = std::max<float>(100.0f, buttonLabelSize.x);

				ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						AssetHandle handle = *(AssetHandle*)payload->Data;
						if (AssetManager::GetAssetType(handle) == AssetType::Texture)
						{
							component.texture = handle;
						}
						else
						{
							// log warning: wrong asset type
						}
					}
					ImGui::EndDragDropTarget();
				}

				DrawIntControl("Layer", component.layer, -1, 10);
			});

		DrawComponentGUI<CBoundingBox>("Box Collider 2D", m_InspectedEntity, [](auto& component)
			{
				DrawVec2Control("Offset", component.offset, 0.0f, 80.0f);
				DrawVec2Control("Size", component.size, 0.0f, 80.0f);
				component.halfSize = component.size / 2;
			});

		DrawComponentGUI<CPolygonCollider>("Polygon Collider 2D", m_InspectedEntity, [](auto& component)
			{
				DrawVec2Control("Offset", component.offset, 0.0f, 80.0f);
				if (ImGui::CollapsingHeader("Points"))
				{
					for (size_t i = 0; i < component.colliderVertices.size(); i++)
					{
						std::string label = "Point " + std::to_string(i);
						DrawVec2Control(label, component.colliderVertices[i], 0.0f, 80.0f);
					}
				}
			});

	}

	ImGui::End();
}

template<typename T, typename... TArgs>
void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName, TArgs&&... mArgs)
{
	if (!m_InspectedEntity.hasComponent<T>())
	{
		if (ImGui::MenuItem(entryName.c_str()))
		{
			m_InspectedEntity.addComponent<T>(std::forward<TArgs>(mArgs)...);
			ImGui::CloseCurrentPopup();
		}
	}
}



void SceneHierarchyPanel::SetInspectedEntity(Entity entity)
{
	m_InspectedEntity = entity;
}

void SceneHierarchyPanel::DrawEntityNode(Entity entity)
{
	auto& tag = entity.getComponent<CTag>().tag;

	ImGuiTreeNodeFlags flags = ((m_InspectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(entity.id()), flags, tag.c_str());
	if (ImGui::IsItemClicked())
	{
		m_InspectedEntity = entity;
	}

	bool entityDeleted = false;
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Delete Entity"))
		{
			entityDeleted = true;
		}
		ImGui::EndPopup();
	}

	if (opened)
	{
		// TODO: add entity info  ?? (id, pos, etc)
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
		if (opened)
		{
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (entityDeleted)
	{
		entity.destroy();
		if (m_InspectedEntity == entity)
		{
			m_InspectedEntity = {};
		}
	}
}

