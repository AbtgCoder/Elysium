#include "SceneHierarchyPanel.h"

#include "Asset/AssetManager.h"
#include "core/Texture.h"
#include "core/Logger.h"
#include "Physics/graham_scan.h"
#include "Utils/StringUtils.h"
#include "../Helper/ImGuiHelper.h"

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
void SceneHierarchyPanel::DrawComponentGUI(const std::string& name, Entity entity, UIFunction uiFunction)
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

	std::vector<Vec2> colliderVertices;
	Vec2 eSize(tex.getSize().x, tex.getSize().y);
	for (auto p : convexHull)
	{
		colliderVertices.push_back(Vec2(- eSize.x / 2 + p.x, eSize.y / 2 - p.y));
	}
	return colliderVertices;
	
}

std::vector<Vec2> generatePolygonColliderVertices(int numSides, float radius)
{
	std::vector<Vec2> vertices;
	vertices.reserve(numSides);

	for (int i = 0; i < numSides; i++)
	{
		float theta = 2.0f * 3.14 * i / numSides;
		float x = radius * std::sin(theta);
		float y = -1 * radius * std::cos(theta);
		vertices.emplace_back(Vec2(x, y));
	}
	return vertices;
}

void SceneHierarchyPanel::OnImGuiRender()
{

	if (m_Scene)
	{
		m_Scene->m_entityManager.update(); // separate update function ??
		
		ImGui::Begin("Scene Hierarchy");

		// Search Bar
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strncpy_s(buffer, sizeof(buffer), searchQuery.c_str(), sizeof(buffer));
		if (ImGui::InputTextEx("##SearchEntity", "Search Entity", buffer, sizeof(buffer), ImVec2(ImGui::GetContentRegionAvail().x, 24), ImGuiInputTextFlags_None))
		{
			searchQuery = std::string(buffer);
		}

		std::vector<Entity> entitiesToDisplay;
		if (searchQuery.empty())
		{
			entitiesToDisplay = m_Scene->m_entityManager.GetEntities();
		}
		else
		{
			//TODO: improve search matching
			for (auto e : m_Scene->m_entityManager.GetEntities())
			{
				auto entityName = e.getComponent<CTag>().tag;
				if (StringUtils::RemoveWhiteSpace(StringUtils::ToUpper(entityName)).find(StringUtils::RemoveWhiteSpace(StringUtils::ToUpper(searchQuery))) != std::string::npos)
				{
					entitiesToDisplay.push_back(e);
				}
			}
		}

		
		for (auto e : entitiesToDisplay)
		{
			if ((searchQuery.empty() && !e.getComponent<CParent>().HasParent) || !searchQuery.empty())
			{
				DrawEntityNode(e);
			}
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_InspectedEntity = {};
		}

		// Right click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			Entity entity;
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				entity = m_Scene->AddEntity("Empty Entity");
			}
			if (ImGui::MenuItem("Create Circle Entity"))
			{
				entity = m_Scene->AddEntity("Circle");
				entity.addComponent<CCircle>();
			}
			if (ImGui::MenuItem("Create Rectangle Entity"))
			{
				entity = m_Scene->AddEntity("rectangle");
				entity.addComponent<CRectangle>();
			}
			if (ImGui::MenuItem("Create Polygon Entity"))
			{
				entity = m_Scene->AddEntity("polygon");
				entity.addComponent<CPolygon>();
			}
			if (entity)
			{
				m_InspectedEntity = entity;
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
			else if (m_InspectedEntity.hasComponent<CPolygon>())
			{
				DisplayAddComponentEntry<CPolygonCollider>("Polygon Collider 2D", generatePolygonColliderVertices(m_InspectedEntity.getComponent<CPolygon>().sides, m_InspectedEntity.getComponent<CPolygon>().size)); // TODO : generate polygon collider vertices from no of sides
			}
			else
			{
				DisplayAddComponentEntry<CCircle>("Circle Shape");
				DisplayAddComponentEntry<CRectangle>("Rectangle Shape");
				DisplayAddComponentEntry<CPolygon>("Polygon Shape");
				DisplayAddComponentEntry<CCircleCollider>("Circle Collider 2D");
				DisplayAddComponentEntry<CBoundingBox>("Box Collider 2D");
				DisplayAddComponentEntry<CPolygonCollider>("Polygon Collider 2D");
			}
			DisplayAddComponentEntry<CRigidBody>("Rigidbody 2D");
			DisplayAddComponentEntry<CJoint>("Joint Component", m_InspectedEntity.getComponent<CId>().id); //TODO: only if entity has rigidbody or add that if not already present
			DisplayAddComponentEntry<CSpriteRenderer>("Sprite Renderer");
			DisplayAddComponentEntry<CPhysicsMaterial>("Physics Material");
			DisplayAddComponentEntry<CNativeScriptComponent>("Native Script");
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();


		DrawComponentGUI<CTransform>("Transform", m_InspectedEntity, [](auto& component)
			{
				DrawVec2Control("Translation", component.Translation, 0.0f, 80.0f);
				DrawFloatControl("Rotation", component.Rotation, 0.0f, 360.0f);
			//	DrawVec2Control("Velocity", component.velocity, 0.0f, 80.0f);
				DrawVec2Control("Scale", component.Scale, 0.0f, 80.0f);
				//DrawFloatControl("Angular velocity", component.angularVelocity, -100.0f, 100.0f, 130.0f);
			});

		DrawComponentGUI<CCamera>("Camera", m_InspectedEntity, [](auto& component)
			{
				DrawVec2Control("Size", component.size, 0.0f, 80.0f);
				DrawFloatControl("Zoom", component.zoom, 0.5f, 2.0f);
				ImGui::Checkbox("Primary", &component.primary);

				float colorArray[4];
				colorArray[0] = component.backgroundColor.r / 255.0f;
				colorArray[1] = component.backgroundColor.g / 255.0f;
				colorArray[2] = component.backgroundColor.b / 255.0f;
				colorArray[3] = component.backgroundColor.a / 255.0f;
				if (ImGui::ColorEdit4("Color", colorArray))
				{
					component.backgroundColor = sf::Color(
						static_cast<sf::Uint8>(colorArray[0] * 255),
						static_cast<sf::Uint8>(colorArray[1] * 255),
						static_cast<sf::Uint8>(colorArray[2] * 255),
						static_cast<sf::Uint8>(colorArray[3] * 255)
					);
				}
			});

		DrawComponentGUI<CNativeScriptComponent>("Native Script", m_InspectedEntity, [](auto& component) 
			{
			});

		DrawComponentGUI<CCircle>("Circle Shape", m_InspectedEntity, [](auto& component) 
			{
				float colorArray[4];
				colorArray[0] = component.color.r / 255.0f;
				colorArray[1] = component.color.g / 255.0f;
				colorArray[2] = component.color.b / 255.0f;
				colorArray[3] = component.color.a / 255.0f;
				if (ImGui::ColorEdit4("Color", colorArray))
				{
					component.color = sf::Color(
						static_cast<sf::Uint8>(colorArray[0] * 255),
						static_cast<sf::Uint8>(colorArray[1] * 255),
						static_cast<sf::Uint8>(colorArray[2] * 255),
						static_cast<sf::Uint8>(colorArray[3] * 255)
					);
				}
				DrawFloatControl("Radius", component.radius, 0.0f, 200.0f);
			});

		DrawComponentGUI<CRectangle>("Rectangle Shape", m_InspectedEntity, [](auto& component)
			{
				float colorArray[4];
				colorArray[0] = component.color.r / 255.0f;
				colorArray[1] = component.color.g / 255.0f;
				colorArray[2] = component.color.b / 255.0f;
				colorArray[3] = component.color.a / 255.0f;
				if (ImGui::ColorEdit4("Color", colorArray))
				{
					component.color = sf::Color(
						static_cast<sf::Uint8>(colorArray[0] * 255),
						static_cast<sf::Uint8>(colorArray[1] * 255),
						static_cast<sf::Uint8>(colorArray[2] * 255),
						static_cast<sf::Uint8>(colorArray[3] * 255)
					);
				}
				DrawVec2Control("Size", component.size, 0.0f, 80.0f);
			});

		DrawComponentGUI<CPolygon>("Polygon Shape", m_InspectedEntity, [](auto& component)
			{
				float colorArray[4];
				colorArray[0] = component.color.r / 255.0f;
				colorArray[1] = component.color.g / 255.0f;
				colorArray[2] = component.color.b / 255.0f;
				colorArray[3] = component.color.a / 255.0f;
				if (ImGui::ColorEdit4("Color", colorArray))
				{
					component.color = sf::Color(
						static_cast<sf::Uint8>(colorArray[0] * 255),
						static_cast<sf::Uint8>(colorArray[1] * 255),
						static_cast<sf::Uint8>(colorArray[2] * 255),
						static_cast<sf::Uint8>(colorArray[3] * 255)
					);
				}
				DrawIntControl("Sides", component.sides, 3, 10);
				DrawFloatControl("Size", component.size, 0.0f, 200.0f);
			});

		DrawComponentGUI<CCircleCollider>("Circle Collider 2D", m_InspectedEntity, [](auto& component)
			{
				DrawFloatControl("Radius", component.radius, 0.0f, 200.0f);
			});

		DrawComponentGUI<CPhysicsMaterial>("Physics Material", m_InspectedEntity, [](auto& component)
			{
				DrawFloatControl("Mass", component.mass, 1.0f, 100.0f);
				DrawFloatControl("Restitution", component.restitutionCoefficient, 0.0f, 1.0f);
				DrawFloatControl("Restitution Threshold", component.restitutionThreshold, 0.0f, 1.0f);
				DrawFloatControl("Friction", component.friction, 0.0f, 1.0f);
			});

		DrawComponentGUI<CRigidBody>("Rigidbody 2D", m_InspectedEntity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic" };
				const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
				if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
						if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
						{
							currentBodyTypeString = bodyTypeStrings[i];
							component.Type = (CRigidBody::BodyType)i;
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
			});

		DrawComponentGUI<CJoint>("Joint Component", m_InspectedEntity, [this](auto& component)
			{
				std::string label = "None (Rigidbody 2D)";
				if (m_Scene->IsEntityUUIDValid(component.entity2Id))
				{
					label = m_Scene->GetEntityByUUID(component.entity2Id).getComponent<CTag>().tag + " (Rigidbody 2D)";
				}

				ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
				buttonLabelSize.x += 20.0f;
				float buttonLabelWidth = std::max<float>(100.0f, buttonLabelSize.x);

				ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
					{
						Elysium::UUID entityId = *(Elysium::UUID*)payload->Data;
						auto e = m_Scene->GetEntityByUUID(entityId);
						if (e.hasComponent<CRigidBody>())
							component.entity2Id = entityId;
					}
					ImGui::EndDragDropTarget();
				}

				DrawVec2Control("Anchor", component.anchorPos, 0.0f, 80.0f);
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
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Image"))
					{
						char* file = (char*)payload->Data;
						std::string fullPath = std::string(file, 256);
						auto relativePath = std::filesystem::relative(fullPath, Project::GetActiveAssetDirectory());
						AssetHandle handle = 0;
						if (!Project::GetActive()->GetEditorAssetManager()->AssetExistsAtFilePath(relativePath))
						{
							Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
						}
						handle = Project::GetActive()->GetEditorAssetManager()->GetAssetHandle(relativePath);
						component.texture = handle;
					}
					else
					{
						// log warning: wrong asset type
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
	else
	{
		// log/display: entity already has component T
	}
}



void SceneHierarchyPanel::SetInspectedEntity(Entity entity)
{
	m_InspectedEntity = entity;
}

void SceneHierarchyPanel::DrawEntityNode(Entity entity)
{
	auto& tag = entity.getComponent<CTag>().tag;

	ImGuiTreeNodeFlags flags = ((m_InspectedEntity.getComponent<CId>().id == entity.getComponent<CId>().id) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

	auto& parent = entity.getComponent<CParent>();

	if (parent.Children.size() <= 0)
		flags |= ImGuiTreeNodeFlags_Leaf; 

	auto cursorPos = ImGui::GetCursorPos();
	ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
	const auto& cleanname = StringUtils::RemoveWhiteSpace(StringUtils::ToUpper(tag));
	const size_t searchIt = cleanname.find(StringUtils::RemoveWhiteSpace(StringUtils::ToUpper(searchQuery)));

	//ImGui::SetItemAllowOverlap();
	bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(entity.getComponent<CId>().id), flags, tag.c_str());

	/*if (!searchQuery.empty() && searchIt != std::string::npos)
	{
		int firstLetterFoundIndex = static_cast<int>(searchIt);
		const auto foundStr = tag.substr(0, firstLetterFoundIndex + searchQuery.size());
		auto highlightBeginPos = ImGui::CalcTextSize(foundStr.c_str());
		auto highlightEndPos = ImGui::CalcTextSize(searchQuery.c_str());

		auto fg = ImGui::GetForegroundDrawList();
		auto rgbColor = IM_COL32(97.0f, 0.0f, 255.0f, 0.2f * 255.0f);
		fg->AddRectFilled(ImVec2(cursorPosition.x + 20.0f, cursorPosition.y + 4.0f), ImVec2(cursorPosition.x + highlightEndPos.x + 26.0f, cursorPosition.y + highlightEndPos.y + 6.0f), rgbColor, 4.0f);
	}*/

	if (ImGui::IsItemHovered())
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			m_MouseButtonPressed = true;
		}

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_MouseButtonPressed)
		{
			m_InspectedEntity = entity;
			m_MouseButtonPressed = false; 
		}
	}

	// Drag drop entity
	if (ImGui::BeginDragDropSource())
	{
		Elysium::UUID entityId = entity.getComponent<CId>().id;
		ImGui::SetDragDropPayload("ENTITY", &entityId, sizeof(Elysium::UUID));
		//TODO: ImGui::Image( (ImTextureID)drag_entity_texture_id, DRAG_ENTITY_SIZE );
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
		{
			Elysium::UUID eId = *(Elysium::UUID*)payload->Data;


			bool entityContainsItself = false; // TODO: implement this function

			auto& payloadParentComponent = m_Scene->GetEntityByUUID(eId).getComponent<CParent>();
			
			if (!entityContainsItself && payloadParentComponent.ParentID != eId && std::count(parent.Children.begin(), parent.Children.end(), eId) == 0)
			{
				if (payloadParentComponent.HasParent)
				{
					// payload_entity already has a parent
					auto& ogParentComponent = m_Scene->GetEntityByUUID(payloadParentComponent.ParentID).getComponent<CParent>(); // it is a child of this parent ...
					// so, remove payload_entity from og_parent
					ogParentComponent.Children.erase(std::remove(ogParentComponent.Children.begin(), ogParentComponent.Children.end(), eId), ogParentComponent.Children.end());
				}
				payloadParentComponent.HasParent = true;
				payloadParentComponent.ParentID = entity.getComponent<CId>().id;
				parent.Children.push_back(eId);
				//TODO OR NOT_TODO: m_Scene->GetEntityByUUID(eId).getComponent<CTransform>().Translation = m_Scene->GetEntityByUUID(eId).getComponent<CTransform>().GlobalTranslation - entity.getComponent<CTransform>().GlobalTranslation;
				//m_InspectedEntity = m_Scene->GetEntityByUUID(eId);
			}
		}
		ImGui::EndDragDropTarget();
	}


	bool entityDeleted = false;
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Delete Entity"))
		{
			entityDeleted = true;
		}
		if (ImGui::MenuItem("Create Empty"))
		{
			Entity child = m_Scene->AddEntity("Empty Entity");
			child.getComponent<CParent>().HasParent = true;
			child.getComponent<CParent>().ParentID = entity.getComponent<CId>().id;
			parent.Children.push_back(child.getComponent<CId>().id);
			m_InspectedEntity = child;
		}
		//TODO: move to root ...
		if (parent.HasParent && ImGui::MenuItem("Move to root"))
		{
			auto& parentParentComponent = m_Scene->GetEntityByUUID(parent.ParentID).getComponent<CParent>();
			parentParentComponent.RemoveChild(entity.getComponent<CId>().id);
			parent.HasParent = false;
		}
		ImGui::EndPopup();
	}


	if (opened)
	{
		// TODO: draw children entities
		auto children = parent.Children;
		for (auto& child : children)
		{
			DrawEntityNode(m_Scene->GetEntityByUUID(child));
		}
		ImGui::TreePop();
	}

	if (entityDeleted)
	{
		m_Scene->DestroyEntity(entity);
		if (m_InspectedEntity == entity)
		{
			m_InspectedEntity = {};
		}
	}
}

