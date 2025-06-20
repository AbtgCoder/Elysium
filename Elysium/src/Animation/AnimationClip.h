#pragma once


#include "SpriteSheet.h" 

struct AnimationFrame
{
	glm::vec2 UVmin; // bottom-left UV
	glm::vec2 UVMax; // top-right UV
	float Duration; // in seconds
};

class AnimationClip : public Asset
{
public:
	AnimationClip() = default;
	AnimationClip(const std::string& name)
		: m_Name(name) {}

	virtual AssetType GetType() const { return AssetType::AnimationClip; }

	void AddFrame(const glm::vec2& uvMin, const glm::vec2& uvMax, float duration);

private: // maybe have the variables private, but have animation state and animation panels as friends
	std::string m_Name = "";
	std::shared_ptr<Texture2D> m_SpriteSheetTexture = nullptr;
	std::string m_SpriteSheetTexturePath = "";
	std::vector<AnimationFrame> m_Frames;

	bool m_Loop = true;

	float m_TotalDuration = 0.0f;

	friend class AnimationController;
	friend class Scene;
	friend class AnimationPanel;
	friend class AnimationSerializer;
};