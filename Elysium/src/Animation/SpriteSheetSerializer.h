#pragma once

#include "SpriteSheet.h"
#include <filesystem>

class SpriteSheetSerializer
{
public:
	SpriteSheetSerializer(const std::shared_ptr<SpriteSheet>& spriteSheet);

	void Serialize(const std::filesystem::path& filepath);

	bool Deserialize(const std::filesystem::path& filepath);

private:
	std::shared_ptr<SpriteSheet> m_SpriteSheet;
};