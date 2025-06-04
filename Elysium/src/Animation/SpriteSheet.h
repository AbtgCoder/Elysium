#pragma once

#include "Renderer/Texture.h"


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <string>
#include <vector>
#include <memory>

struct SubSprite
{
	std::string name;
	glm::vec2 position; // top-left corner in pixels
	glm::vec2 size; // width and height in pixels
	// pivot
	// uvs for rendering
};

class SpriteSheet : public Asset
{
public:
	SpriteSheet() = default;

	virtual AssetType GetType() const { return AssetType::SpriteSheet; }

	std::shared_ptr<Texture2D> GetTexture() { return m_Texture; }
private:
	std::shared_ptr<Texture2D> m_Texture = nullptr;
	std::vector<SubSprite> m_SubSprites;
	std::string m_SourcePath;

	friend class SpriteSheetEditorPanel;
	friend class SpriteSheetSerializer;
};