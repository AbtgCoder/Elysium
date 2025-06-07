#include "AnimationPanel.h"
#include <Utils/FileUtils.h>

#include "core/Logger.h"

#include "Asset/AnimationImporter.h"
#include "Asset/AssetManager.h"
#include "Project/Project.h"
#include <filesystem>

void AnimationPanel::SetInspectedEntity(Entity entity)
{
	m_InspectedEntity = entity;
}

void AnimationPanel::CreateAnimationClip()
{
	if (m_InspectedEntity && m_InspectedEntity.hasComponent<CAnimator>())
	{
		// create animation clip file ig, and create animation clip here and add it to animator's state...
		auto& animController = m_InspectedEntity.getComponent<CAnimator>().Controller;
		std::string path = WindowsFileUtils::SaveFile("Animation clip (*.esmanim)\0*.esmanim\0");
		if (!path.empty())
		{
			//TODO: really feel like there should be a better way/design to handle all this...
			auto clip = std::make_shared<AnimationClip>(std::filesystem::path(path).stem().string());
			auto relativePath = std::filesystem::relative(path, Project::GetActiveAssetDirectory());
			AnimationImporter::SaveAnimationClip(clip, relativePath);
			Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
			auto actualAssetHandle = Project::GetActive()->GetEditorAssetManager()->GetAssetHandle(relativePath);
			clip->Handle = actualAssetHandle; //NOTE: it is neccessary to store the actual asset handle, when storing the clip in animator, as this is what will be serialized and used to deserialize with scene having entities with animations..
			animController.AddState(std::filesystem::path(path).stem().string(), clip);
		}
	}
}

static float sidebarWidth = 200.0f;
static float topBarHeight = 40.0f;

void AnimationPanel::OnImGuiRender()
{
	if (!m_Open)
		return;

	ImGui::Begin("Animation", &m_Open);

	if (m_InspectedEntity)
	{
		if (!m_InspectedEntity.hasComponent<CAnimator>())
		{
			std::string text = "To begin Animating " + m_InspectedEntity.getComponent<CTag>().tag + ", create an Animator and an Animation clip.";

			ImGui::TextWrapped(text.c_str());

			if (ImGui::Button("Create"))
			{
				m_InspectedEntity.addComponent<CAnimator>();
				
				CreateAnimationClip();
			}
		}
		else
		{
			// draw UI

			//if (ImGui::BeginChild("##topbar"), ImVec2(0, topBarHeight), false, ImGuiWindowFlags_NoScrollbar)
			//{
			//	//TODO: have play/pause etc buttons here
			//	ImGui::Text("Preview");

			//	ImGui::SameLine();
			//	
			//	if (ImGui::Button("Play"))
			//	{

			//	}
			//}
			//ImGui::EndChild();

			ImGui::Separator();

			auto& animController = m_InspectedEntity.getComponent<CAnimator>().Controller;

			ImGui::BeginChild("##leftpanel", ImVec2(sidebarWidth, 0), false);
			{
				ImGui::Text("Animation States");
				ImGui::Separator();

				static int selectedStateIndex = -1;
				int i = 0;
				for (auto& [stateName, state] : animController.m_States)
				{
					bool isSelected = (i == selectedStateIndex);
					if (ImGui::Selectable(state.Clip->m_Name.c_str(), isSelected))
					{
						selectedStateIndex = i;
						animController.m_CurrentState = stateName;
					}
					i++;
				}

				if (ImGui::Button("+ Create New Clip"))
				{
					CreateAnimationClip();

				}
				//TODO: add an exisiting clip...
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("##rightpanel", ImVec2(0, 0), false);
			{
				ImGui::Text("Animation Timeline");
				ImGui::Separator();

				if (!animController.m_CurrentState.empty() && animController.m_States.contains(animController.m_CurrentState))
				{
					auto& clip = animController.m_States[animController.m_CurrentState].Clip;
					if (clip)
					{
						ImGui::Text("Clip: %s", clip->m_Name.c_str());
						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Spritesheet"))
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
								auto spriteSheet = AssetManager::GetAsset<SpriteSheet>(handle);

								clip->m_SpriteSheetTexture = spriteSheet->GetTexture();
								clip->m_SpriteSheetTexturePath = spriteSheet->GetTexturePath();

								auto texWidth = clip->m_SpriteSheetTexture->GetWidth();
								auto texHeight = clip->m_SpriteSheetTexture->GetHeight();

								float frameDuration = 1.0f / 12.0f; //TODO: maybe should be a default parameter in clip->addframe

								for (auto subsprite : spriteSheet->m_SubSprites)
								{
									glm::vec2 uvMin = {
										subsprite.position.x / texWidth,
										subsprite.position.y / texHeight
									};

									glm::vec2 uvMax = {
										(subsprite.position.x + subsprite.size.x) / texWidth,
										((subsprite.position.y + subsprite.size.y) / texHeight)
									};
									clip->AddFrame(uvMin, uvMax, frameDuration);
								}
								
								Logger::Log("Added spritesheet to animaiton clip", "editor");
							}
							//TODO: accept drag drop payload "animation clip"....
							ImGui::EndDragDropTarget();

						}
					
						ImGui::Checkbox("Loop", &clip->m_Loop);
						ImGui::Text("Total Duration: %.4f sec", clip->m_TotalDuration);
						ImGui::Separator();

						ImGui::Text("Timeline:");

						ImGui::BeginChild("##TimelineEditor", ImVec2(0, 200), true);
						ImGui::Columns((int)clip->m_Frames.size(), nullptr, false);
						for (size_t i = 0; i < clip->m_Frames.size(); ++i)
						{
							ImGui::PushID((int)i);

							// draw thumbnail
							if (clip->m_SpriteSheetTexture)
							{
								float w = 32.0f, h = 32.0f;
								auto uvMin = clip->m_Frames[i].UVmin;
								auto uvMax = clip->m_Frames[i].UVMax;
								ImGui::Image((ImTextureID)clip->m_SpriteSheetTexture->GetRendererID(), ImVec2(w, h), { uvMin.x, uvMax.y }, { uvMax.x, uvMin.y });
								ImGui::Text("%.4fs", clip->m_Frames[i].Duration);
							}

							// allow changing the duration here
							char label[32];
							sprintf(label, "Frame %d##Duration", (int)i);
							ImGui::SliderFloat(label, &clip->m_Frames[i].Duration, 0.01f, 0.5f, "%.4fs");

							if (ImGui::Button("Delete"))
							{
								clip->m_TotalDuration -= clip->m_Frames[i].Duration;
								clip->m_Frames.erase(clip->m_Frames.begin() + i);
								ImGui::PopID();
								ImGui::NextColumn();
								break;
							}
							ImGui::PopID();
							ImGui::NextColumn();
						}
						ImGui::Columns(1);
						ImGui::EndChild();

						if (ImGui::Button("+ Add Empty Frame"))
						{
							clip->AddFrame({ 0.0f, 0.0f }, { 1.0f, 1.0f }, 1.0f / 12.0f);
						}
					}
				}
				else
				{
					ImGui::Text("No valid animation clip selected.");
				}
			}
			ImGui::EndChild();
		}
	}

	ImGui::End();
}