#include "AnimationPanel.h"
#include <Utils/FileUtils.h>

#include "core/Logger.h"

#include "Asset/TextureImporter.h"
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

				int i = 0;
				for (auto& [stateName, state] : animController.m_States)
				{
					bool isSelected = (i == m_SelectedStateIndex);
					if (ImGui::Selectable(state.Clip->m_Name.c_str(), isSelected))
					{
						m_SelectedStateIndex = i;
						animController.m_CurrentState = stateName;
					}
					i++;
				}

				ImGui::Separator();
				ImGui::Text("Parameters");
				ImGui::Separator();

				for (auto& [name, param] : animController.m_Parameters)
				{
					ImGui::PushID(name.c_str());

					ImGui::Text("%s", name.c_str());
					ImGui::SameLine(120);

					if (param.Type == AnimatorParameterType::Bool)
					{
						ImGui::Checkbox("##bool", &param.BoolValue);
					}
					else if (param.Type == AnimatorParameterType::Float)
					{
						ImGui::DragFloat("##float", &param.FloatValue, 0.1f);
					}
					else if (param.Type == AnimatorParameterType::Trigger)
					{
						if (ImGui::Button("Fire"))
							param.Triggered = true;
					}

					ImGui::PopID();
				}

				ImGui::Separator();
				ImGui::InputText("Name", m_NewParameterName, sizeof(m_NewParameterName));

				const char* types[] = { "Bool", "Float", "Trigger" };
				ImGui::Combo("Type", &m_NewParameterType, types, IM_ARRAYSIZE(types));

				if (ImGui::Button("+ Add Parameter"))
				{
					if (strlen(m_NewParameterName) > 0)
					{
						AnimatorParameterType type = (AnimatorParameterType)m_NewParameterType;
						animController.AddParameter(m_NewParameterName, type);
						memset(m_NewParameterName, 0, sizeof(m_NewParameterName));
					}
				}

				ImGui::Separator();
				ImGui::Separator();

				if (ImGui::Button("+ Create New Clip"))
				{
					CreateAnimationClip();

				}
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
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Image"))
							{
								char* file = (char*)payload->Data;
								std::string fullPath = std::string(file, 256);
								auto relativePath = std::filesystem::relative(fullPath, Project::GetActiveAssetDirectory());

								clip->m_SpriteSheetTexture = TextureImporter::LoadTexture2D(fullPath);
								clip->m_SpriteSheetTexturePath = relativePath.string();

								clip->AddFrame({ 0.0f, 0.0f }, { 1.0f, 1.0f }, 1.0f / 12.0f); // one single frame

								Logger::Log("Added sprite to animaiton clip", "editor");
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

				ImGui::Separator();
				ImGui::Text("Transitions");
				ImGui::Separator();

				if (!animController.m_CurrentState.empty())
				{
					auto& state = animController.m_States[animController.m_CurrentState];

					int tIndex = 0;
					for (auto& transition : state.Transitions)
					{
						ImGui::PushID(tIndex);

						bool selected = (m_SelectedTransitionIndex == tIndex);
						std::string label = "-> " + transition.ToState;

						if (ImGui::Selectable(label.c_str(), selected))
							m_SelectedTransitionIndex = tIndex;

						ImGui::PopID();
						tIndex++;
					}

					if (m_SelectedTransitionIndex >= 0 && m_SelectedTransitionIndex < state.Transitions.size())
					{
						auto& transition = state.Transitions[m_SelectedTransitionIndex];

						ImGui::Separator();
						ImGui::Text("Conditions");

						int cIndex = 0;
						for (auto& condition : transition.Conditions)
						{
							ImGui::PushID(cIndex);

							auto& parameterName = condition.ParameterName;
							char buffer[256];
							memset(buffer, 0, sizeof(buffer));
							strncpy_s(buffer, sizeof(buffer), parameterName.c_str(), sizeof(buffer));
							if (ImGui::InputText("##parameter", buffer, sizeof(buffer)))
							{
								parameterName = std::string(buffer);
							}

							const char* condTypes[] =
							{
								"BoolTrue",
								"BoolFalse",
								"FloatGreater",
								"FloatLess",
								"Trigger"
							};
							
							int condType = (int)condition.ConditionType;
							ImGui::Combo("Condition", &condType, condTypes, IM_ARRAYSIZE(condTypes));
							condition.ConditionType = (AnimatorConditionType)condType;

							if (condition.ConditionType == AnimatorConditionType::FloatGreater || condition.ConditionType == AnimatorConditionType::FloatLess)
							{
								ImGui::DragFloat("Threshold", &condition.Threshold, 0.1f);
							}

							if (ImGui::Button("Delete Condition"))
							{
								transition.Conditions.erase(transition.Conditions.begin() + cIndex);
								ImGui::PopID();
								break;
							}

							ImGui::PopID();
							cIndex++;
						}

						if (ImGui::Button("+ Add Condition"))
						{
							AnimatorCondition c;
							transition.Conditions.push_back(c);
						}

						/*ImGui::Separator();
						ImGui::Checkbox("Has Exit Time", &transition.HasExitTime);
						ImGui::DragFloat("Exit Time", &transition.ExitTime, 0.01f, 0.0f, 1.0f);*/
					}


					ImGui::Separator();
					ImGui::InputText("To State", m_NewTransitionToState, sizeof(m_NewTransitionToState)); //TODO: problem is user has to manually type in the name of the "ToState" which could be wrong, ig we need to make a proper node-graph editor...
					if (ImGui::Button("+ Add Transition"))
					{
						if (strlen(m_NewTransitionToState) > 0)
						{
							AnimationTransition t;
							t.FromState = animController.m_CurrentState;
							t.ToState = m_NewTransitionToState;

							state.Transitions.push_back(t);
							memset(m_NewTransitionToState, 0, sizeof(m_NewTransitionToState));
						}
					}
				}
			}
			ImGui::EndChild();
		}
	}

	ImGui::End();
}